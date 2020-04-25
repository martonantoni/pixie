#pragma once

class cPixieObjectRotator: public cPixieObjectAnimator
{
public:
	enum eDirection { ClockWise, CounterClockWise };
private:
	unsigned int mTimeForFullCircle;
	float mStartRotation=0.0;
    float mTargetSumRotation;
	eDirection mDirection;
protected:
	virtual void Activated(cPixieObject &Object) override;
	virtual eAnimateResult Animate(cPixieObject &Object) override;
public:
	cPixieObjectRotator(unsigned int TimeForFullCircle, eDirection Direction, float TargetSumRotation);

    static tIntrusivePtr<cPixieObjectRotator> CreateEndless(unsigned int TimeForFullCircle, eDirection Direction)
    {
        return make_intrusive_ptr<cPixieObjectRotator>(TimeForFullCircle, Direction, -1.0);
    }
};
