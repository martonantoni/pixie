#include "StdAfx.h"

cGeneralPixieObjectBlender::cGeneralPixieObjectBlender(const cRequest &Request)
	: cPixieObjectAnimator
		(DoneWhenSamePropertyAnimatorAdded|DoneWhenAffectedPropertySet|(Request.mKeepObjectAlive?KeepsObjectAlive:0),Request.mAffectedProperties)
	, mRequest(Request)
{
	SetStartTime(mRequest.mStartTime);
}

void cGeneralPixieObjectBlender::BlendObject(cPixieObject &Object, const cPixieObject::cPropertyValues &TargetValues, unsigned int AffectedProperties, unsigned int BlendTime, bool KeepObjectAlive)
{
	cRequest Request;
	Request.mBlendTime=BlendTime;
	Request.mTargetValues=TargetValues;
	Request.mAffectedProperties=AffectedProperties;
	Request.mKeepObjectAlive=KeepObjectAlive;
	Object.AddAnimator(make_intrusive_ptr<cGeneralPixieObjectBlender>(Request));
}

void cGeneralPixieObjectBlender::Activated(cPixieObject &Object)
{
	if(ASSERTFALSE(!Object.GetProperty(mRequest.mAffectedProperties, mStartValues)))
	{
		return;
	}
	ASSERT(mRequest.mTargetValues.mPropertyCount==mStartValues.mPropertyCount);
}

cPixieObjectAnimator::eAnimateResult cGeneralPixieObjectBlender::Animate(cPixieObject &Object)
{
	unsigned int TimeElapsed=gFrameTime-GetStartTime();
	if(TimeElapsed>=mRequest.mBlendTime)
	{
		Object.SetProperty(mRequest.mAffectedProperties, mRequest.mTargetValues);
		return AnimationDone;
	}
	cPixieObject::cPropertyValues NewPropertyValues;
	NewPropertyValues.mPropertyCount=mRequest.mTargetValues.mPropertyCount;
	for(size_t i=0; i<NewPropertyValues.mPropertyCount; ++i)
	{
		NewPropertyValues.mProperties[i]=
			mStartValues.mProperties[i]+((mRequest.mTargetValues.mProperties[i]-mStartValues.mProperties[i])*int(TimeElapsed))/int(mRequest.mBlendTime);
	}
	Object.SetProperty(mRequest.mAffectedProperties, NewPropertyValues);
	return AnimationActive;
}
