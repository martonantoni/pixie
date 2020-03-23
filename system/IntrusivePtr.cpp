#include "StdAfx.h"

void cIntrusiveThreadsafeRefCount::DestroySelf() const
{
	delete this;
}

void cIntrusiveThreadsafeRefCount::ReferenceCounterReachedZero() const
{
	if(mDestructorThread&&!mDestructorThread->IsInThread())
	{
		::CallBack(mDestructorThread, eCallbackType::Normal, this, &cIntrusiveThreadsafeRefCount::DestroySelf);
		return;
	}
	DestroySelf();
}

void cIntrusiveThreadsafeRefCount::PostDestroySelf(cThread *Thread) const
{
	::CallBack(Thread, eCallbackType::NoImmediate, this, &cIntrusiveThreadsafeRefCount::DestroySelf);
}
