#include "StdAfx.h"

cPixieObjectRotator::cPixieObjectRotator(unsigned int TimeForFullCircle, eDirection Direction)
	: cPixieObjectAnimator(DoneWhenSamePropertyAnimatorAdded|DoneWhenAffectedPropertySet, cPixieObject::Property_Rotation)
	, mTimeForFullCircle(TimeForFullCircle)
	, mDirection(Direction)
{
}

cPixieObjectAnimator::eAnimateResult cPixieObjectRotator::Animate(cPixieObject &Object)
{
	auto ElapsedTime=gFrameTime-GetStartTime();
	auto ElapsedCircleTime=ElapsedTime%mTimeForFullCircle;
	if(mDirection==ClockWise)
		Object.SetFloatProperty(cPixieObject::Property_Rotation, mStartRotation+ElapsedCircleTime*360.0/mTimeForFullCircle);
	else
		Object.SetFloatProperty(cPixieObject::Property_Rotation, mStartRotation-ElapsedCircleTime*360.0/mTimeForFullCircle);
	return cPixieObjectAnimator::AnimationActive;
}

void cPixieObjectRotator::Activated(cPixieObject &Object)
{
	Object.GetFloatProperty(cPixieObject::Property_Rotation, mStartRotation);
}
