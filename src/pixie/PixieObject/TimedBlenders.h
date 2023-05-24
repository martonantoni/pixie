#pragma once

class cTimedAnimators: public cPixieObjectAnimator
{
public:
	struct cTimeRange
	{
		unsigned int mStartTime;
		unsigned int mEndTime;
		cTimeRange(unsigned int StartTime, unsigned int EndTime): mStartTime(StartTime), mEndTime(EndTime) {}
		cTimeRange(int StartTime, int EndTime): mStartTime(StartTime), mEndTime(EndTime) {}
		unsigned int Duration() const { return mEndTime-mStartTime; }
	};
	struct cStandardBlenderRequest
	{
		cTimeRange mTiming; // this is relative to the activation of the cTimedBlenders
		unsigned int mAffectedProperties;
		cPixieObject::cPropertyValues mTargetValues;
	};
private:
	struct cScheduledAnimator
	{
		cTimeRange mTiming;
		tIntrusivePtr<cPixieObjectAnimator> mAnimator;
		cScheduledAnimator(const cTimeRange &Timing, tIntrusivePtr<cPixieObjectAnimator> Animator): mTiming(Timing), mAnimator(std::move(Animator)) {}
	};
	unsigned int mLoopAt=0, mLoopTo=0; // if mLoopAt is 0, it means that looping is disabled
	typedef std::vector<cScheduledAnimator> cScheduledAnimators;
	cScheduledAnimators mScheduledAnimators; // ordered in a way that ScheduledBlenders with lower mStartTime are at the back (so we are popping from back)
	cScheduledAnimators mPassedAnimators;    // used only if looping is enabled
	typedef std::vector<tIntrusivePtr<cPixieObjectAnimator>> cActiveAnimators;
	cActiveAnimators mActiveAnimators;
	virtual void Activated(cPixieObject &Object) override;
	virtual eAnimateResult Animate(cPixieObject &Object) override;
	void HandleScheduledAnimators(cPixieObject &Object);
public:
	cTimedAnimators();
	void AddStandardBlender(const cStandardBlenderRequest &BlenderRequest); // must not be called after Activate()
	static tIntrusivePtr<cPixieObjectAnimator> CreateStandardBlender(const cStandardBlenderRequest &ScheduledBlender);
	void AddAnimator(const cTimeRange &Timing, tIntrusivePtr<cPixieObjectAnimator> Animator);
	void SetLooping(unsigned int LoopAt, unsigned int LoopTo); // in ms
	// looping will only activate scheduled blenders if their StartTime is at or after LoopTo (so for example if you set LoopTo 100, and
	//    you have a scheduled blender with mStartTime=50, mEndTime=150 => it will not get activated)
	// if there is any active blender at LoopAt, they will be terminated immediately.
	// must not be called after Activate()
	bool IsLoopingEnabled() const { return mLoopAt!=0; }
	void Activate();
};
