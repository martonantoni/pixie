#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"/*
cAlongPathPixieObjectBlender::cAlongPathPixieObjectBlender(const cRequest &Request)
	: cPixieObjectAnimator
		(DoneWhenSamePropertyAnimatorAdded|DoneWhenAffectedPropertySet|(Request.mKeepObjectAlive?KeepsObjectAlive:0), Request.mAffectedProperties)
	, mRequest(Request)
{
	SetStartTime(mRequest.mStartTime);
}

void cAlongPathPixieObjectBlender::BlendObject(cPixieObject& Object, cPath path, unsigned int AffectedProperties, bool KeepObjectAlive)
{
    if (path.empty())
    {
        return;
    }
	cRequest Request;
	Request.mPath=std::move(path);
	Request.mAffectedProperties=AffectedProperties;
	Request.mKeepObjectAlive=KeepObjectAlive;
	Object.AddAnimator(make_intrusive_ptr<cAlongPathPixieObjectBlender>(Request));
}

void cAlongPathPixieObjectBlender::Activated(cPixieObject &Object)
{
	if(ASSERTFALSE(!Object.GetProperty(mRequest.mAffectedProperties, mStartValues)))
	{
		return;
	}
}

cPixieObjectAnimator::eAnimateResult cAlongPathPixieObjectBlender::Animate(cPixieObject &Object)
{
	int elapsedTime= static_cast<int>(gFrameTime-GetStartTime());
    auto i = std::lower_bound(ALL(mRequest.mPath), elapsedTime, [](auto& point, int t) { return point.first < t; });
	if(i==mRequest.mPath.end())
	{
		Object.SetProperty(mRequest.mAffectedProperties, mRequest.mPath.back().second);
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
*/