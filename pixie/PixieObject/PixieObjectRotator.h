#pragma once

class cPixieObjectRotator: public cPixieObjectAnimator
{
public:
	enum eDirection { ClockWise, CounterClockWise };
private:
	unsigned int mTimeForFullCircle;
	float mStartRotation=0.0;
	eDirection mDirection;
protected:
	virtual void Activated(cPixieObject &Object) override;
	virtual eAnimateResult Animate(cPixieObject &Object) override;
public:
	cPixieObjectRotator(unsigned int TimeForFullCircle, eDirection Direction);
};
