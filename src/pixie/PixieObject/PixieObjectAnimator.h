#pragma once

class cPixieObject;
enum class ePixieObjectAnimationDoneReason
{
	StillRunning, NormalDone, AutoDone
};

class cPixieObjectAnimator: public cIntrusiveRefCount
{
public:
	enum // flags
	{
		DoneWhenAffectedPropertySet        = 0x01,
		DoneWhenSamePropertyAnimatorAdded  = 0x02,
		KeepsObjectAlive                   = 0x08,
	};
	typedef ePixieObjectAnimationDoneReason eAnimationDoneReason;
private:
	std::function<void()> mDoneFunction;
	unsigned int mStartTime=gFrameTime;
protected:
	const unsigned int mFlags;
	unsigned int mAffectedProperties;
	unsigned int GetStartTime() const { return mStartTime; }
public:
	cPixieObjectAnimator(unsigned int Flags, unsigned int AffectedProperties);
	cPixieObjectAnimator(const cPixieObjectAnimator &)=delete;
	virtual ~cPixieObjectAnimator();
	void SetStartTime(unsigned int StartTime) { mStartTime=StartTime; }
	unsigned int GetFlags() const { return mFlags; }
	unsigned int GetAffectedProperties() const { return mAffectedProperties; }

	virtual void Activated(cPixieObject &Object)=0;
	enum eAnimateResult { AnimationDone, AnimationActive };
	virtual eAnimateResult Animate(cPixieObject &Object)=0;
	void SetDoneFunction(const std::function<void()> &DoneFunction);
};

class cEmptyAnimator: public cPixieObjectAnimator
{
public:
	cEmptyAnimator(): cPixieObjectAnimator(0u, 0u) {}
	cEmptyAnimator(const std::function<void()> &DoneFunction): cEmptyAnimator()
	{
		SetDoneFunction(DoneFunction);
	}
	virtual eAnimateResult Animate(cPixieObject &Object) override { return cPixieObjectAnimator::AnimationDone; }
	virtual void Activated(cPixieObject &Object) override {}
};

class cPixieObjectAnimatorManager: public tSingleton<cPixieObjectAnimatorManager>
{
	tRegisteredObjects<cPixieObject *> mAnimatedObjects;
	cRegisteredID mLogicID;
	void OnLogic();
public:
	cPixieObjectAnimatorManager();
	~cPixieObjectAnimatorManager();
	
	cRegisteredID RegisterAnimatedObject(cPixieObject *Object);
};

extern cPixieObjectAnimatorManager *thePixieObjectAnimatorManager;