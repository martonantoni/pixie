#include "StdAfx.h"

cTimedAnimators::cTimedAnimators()
	: cPixieObjectAnimator(DoneWhenSamePropertyAnimatorAdded|DoneWhenAffectedPropertySet|KeepsObjectAlive, 0)
{
}

void cTimedAnimators::Activate()
{
	ASSERT(false);
}

void cTimedAnimators::SetLooping(unsigned int LoopAt, unsigned int LoopTo)
{
	ASSERT(LoopTo<LoopAt);
	ASSERT(LoopAt>0);
	mLoopAt=LoopAt;
	mLoopTo=LoopTo;
}

void cTimedAnimators::AddStandardBlender(const cStandardBlenderRequest &BlenderRequest)
{
	ASSERT(BlenderRequest.mTiming.mEndTime>=BlenderRequest.mTiming.mStartTime); // if this triggers, you probably supplied duration instead of endtime.
	AddAnimator(BlenderRequest.mTiming, CreateStandardBlender(BlenderRequest));
}

void cTimedAnimators::AddAnimator(const cTimeRange &Timing, tIntrusivePtr<cPixieObjectAnimator> Animator)
{
	ASSERT(Timing.mEndTime>=Timing.mStartTime); // if this triggers, you probably supplied duration instead of endtime.
	if(ASSERTFALSE(!Animator))
		return;
	mAffectedProperties|=Animator->GetAffectedProperties();
	mScheduledAnimators.emplace_back(Timing, std::move(Animator));
}

tIntrusivePtr<cPixieObjectAnimator> cTimedAnimators::CreateStandardBlender(const cStandardBlenderRequest &ScheduledBlender)
{
	cGeneralPixieObjectBlender::cRequest Request;
	Request.mBlendTime=ScheduledBlender.mTiming.Duration();
	Request.mAffectedProperties=ScheduledBlender.mAffectedProperties;
	Request.mTargetValues=ScheduledBlender.mTargetValues;
	return make_intrusive_ptr<cGeneralPixieObjectBlender>(Request);
}

void cTimedAnimators::HandleScheduledAnimators(cPixieObject &Object)
{
	unsigned int TimeSinceStart=gFrameTime-GetStartTime();
	if(IsLoopingEnabled()&&TimeSinceStart>mLoopAt)
		TimeSinceStart=mLoopAt;
	while(!mScheduledAnimators.empty()&&mScheduledAnimators.back().mTiming.mStartTime<=TimeSinceStart)
	{
		auto &ScheduledBlender=mScheduledAnimators.back();
		auto Animator=ScheduledBlender.mAnimator;
		Animator->SetStartTime(GetStartTime()+ScheduledBlender.mTiming.mStartTime);
		Animator->Activated(Object);
		if(Animator->Animate(Object)==cPixieObjectAnimator::AnimationActive)
			mActiveAnimators.emplace_back(std::move(Animator));
		if(IsLoopingEnabled())
			mPassedAnimators.push_back(ScheduledBlender);
		mScheduledAnimators.pop_back();
	}
}

void cTimedAnimators::Activated(cPixieObject &Object)
{
	std::stable_sort(mScheduledAnimators, [](auto &a, auto &b)
	{
		return a.mTiming.mStartTime>b.mTiming.mStartTime||
			(a.mTiming.mStartTime==b.mTiming.mStartTime&&a.mTiming.mStartTime!=a.mTiming.mEndTime&&b.mTiming.mStartTime==b.mTiming.mEndTime);
		// we're popping from the back, so the lowest start time is at the back. in case of identical start times, the one with 0 duration is at the back.
	});
	if(IsLoopingEnabled())
		const_cast<unsigned int &>(mFlags)&=~KeepsObjectAlive;
	SetStartTime(gFrameTime);
}

cPixieObjectAnimator::eAnimateResult cTimedAnimators::Animate(cPixieObject &Object)
{
// 1) handle the active blenders
	mActiveAnimators.erase(std::remove_if(mActiveAnimators, 
		[&Object](auto &Blender) { return Blender->Animate(Object)==cPixieObjectAnimator::AnimationDone; }), mActiveAnimators.end());
// 2) handle the scheduled blenders
	HandleScheduledAnimators(Object);
// 3) Handle looping:
	if(IsLoopingEnabled()&&gFrameTime-GetStartTime()>=mLoopAt) // Time since start
	{
		mActiveAnimators.clear();
		while(!mPassedAnimators.empty())
		{
			mScheduledAnimators.push_back(mPassedAnimators.back());
			mPassedAnimators.pop_back();
		}
		auto TimePastLoopAt=(gFrameTime-GetStartTime())-mLoopAt;
		TimePastLoopAt%=mLoopAt-mLoopTo;
		SetStartTime(gFrameTime-mLoopTo-TimePastLoopAt);
		HandleScheduledAnimators(Object);
	}
	return mScheduledAnimators.empty()&&mActiveAnimators.empty()&&!IsLoopingEnabled()?AnimationDone:AnimationActive;
}
