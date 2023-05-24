#include "StdAfx.h"

cPixieObjectRotator::cPixieObjectRotator(unsigned int TimeForFullCircle, eDirection Direction, float TargetSumRotation)
	: cPixieObjectAnimator(DoneWhenSamePropertyAnimatorAdded|DoneWhenAffectedPropertySet, cPixieObject::Property_Rotation)
	, mTimeForFullCircle(TimeForFullCircle)
	, mDirection(Direction)
    , mTargetSumRotation(TargetSumRotation)
{
}

cPixieObjectAnimator::eAnimateResult cPixieObjectRotator::Animate(cPixieObject& Object)
{
    auto ElapsedTime = gFrameTime - GetStartTime();
    float fullRotation = ElapsedTime * 360.0 / mTimeForFullCircle;
    bool done = fullRotation >= mTargetSumRotation;
    auto ElapsedCircleTime = ElapsedTime % mTimeForFullCircle;
    if (mDirection == ClockWise)
        Object.SetFloatProperty(cPixieObject::Property_Rotation, mStartRotation + (done ? mTargetSumRotation : ElapsedCircleTime * 360.0 / mTimeForFullCircle));
    else
        Object.SetFloatProperty(cPixieObject::Property_Rotation, mStartRotation - (done ? mTargetSumRotation : ElapsedCircleTime * 360.0 / mTimeForFullCircle));
    return done ? cPixieObjectAnimator::AnimationDone : cPixieObjectAnimator::AnimationActive;
}

void cPixieObjectRotator::Activated(cPixieObject &Object)
{
	Object.GetFloatProperty(cPixieObject::Property_Rotation, mStartRotation);
}
