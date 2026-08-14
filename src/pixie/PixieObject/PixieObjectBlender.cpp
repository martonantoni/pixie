#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

cGeneralPixieObjectBlender::cGeneralPixieObjectBlender(const cRequest &Request)
	: cPixieObjectAnimator
		(DoneWhenSamePropertyAnimatorAdded|DoneWhenAffectedPropertySet|(Request.mKeepObjectAlive?KeepsObjectAlive:0),Request.mAffectedProperties)
	, mRequest(Request)
{
	SetStartTime(mRequest.mStartTime);
}

tIntrusivePtr<cPixieObjectAnimator> cGeneralPixieObjectBlender::BlendObject(cPixieObject &Object, const cPixieObject::cPropertyValues &TargetValues, unsigned int AffectedProperties, unsigned int BlendTime, bool KeepObjectAlive)
{
	cRequest Request;
	Request.mBlendTime=BlendTime;
	Request.mTargetValues=TargetValues;
	Request.mAffectedProperties=AffectedProperties;
	Request.mKeepObjectAlive=KeepObjectAlive;
	auto animator = make_intrusive_ptr<cGeneralPixieObjectBlender>(Request);
	Object.AddAnimator(animator);
    return animator;
}

void cGeneralPixieObjectBlender::Activated(cPixieObject &Object)
{
	if(ASSERTFALSE(!Object.GetProperty(mRequest.mAffectedProperties, mStartValues)))
	{
		return;
	}
}

cPixieObjectAnimator::eAnimateResult cGeneralPixieObjectBlender::Animate(cPixieObject &Object)
{
	unsigned int TimeElapsed=gFrameTime-GetStartTime();
	if(TimeElapsed>=mRequest.mBlendTime)
	{
		Object.SetProperty(mRequest.mAffectedProperties, mRequest.mTargetValues);
		return AnimationDone;
	}
	cPixieObject::cPropertyValues NewPropertyValues = cPixieObject::cPropertyValues::lerp(mStartValues, mRequest.mTargetValues, float(TimeElapsed) / float(mRequest.mBlendTime));
	Object.SetProperty(mRequest.mAffectedProperties, NewPropertyValues);
	return AnimationActive;
}

