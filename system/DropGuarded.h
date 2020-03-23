#pragma once

class cDropGuarded
{
	friend class cDropGuard;
	bool mIsDropped=false;
	unsigned int mDropGuardRecursionCounter=0;
	cThread *mDropThread=nullptr;
protected:
	virtual ~cDropGuarded()=default;
	bool IsDropped() const { return mIsDropped; }
	void SetDropThread(cThread *Thread) { mDropThread=Thread; }
public:
	void Drop()
	{
		if(mDropThread&&!mDropThread->IsInThread())
		{
			::CallBack(mDropThread, eCallbackType::Normal, this, &cDropGuarded::Drop);
			return;
		}
		mIsDropped=true;
		if(!mDropGuardRecursionCounter)
			delete this;
	}
};

USE_DROP_INSTEAD_DELETE(cDropGuarded);

class cDropGuard
{
	cDropGuarded *mGuardedObj;
public:
	cDropGuard(cDropGuarded *GuardedObj): mGuardedObj(GuardedObj)
	{
		++mGuardedObj->mDropGuardRecursionCounter;
	}
	~cDropGuard()
	{
		if(--mGuardedObj->mDropGuardRecursionCounter&&mGuardedObj->mIsDropped)
			delete mGuardedObj;
	}
};
