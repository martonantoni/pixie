#include "StdAfx.h"

void cIntrusiveThreadsafeRefCount::DestroySelf() const
{
	delete this;
}

void cIntrusiveThreadsafeRefCount::ReferenceCounterReachedZero() const
{
	if(mDestructorThread&&!mDestructorThread->IsInThread())
	{
		mDestructorThread->callback([this]() {DestroySelf(); });
		return;
	}
	DestroySelf();
}

void cIntrusiveThreadsafeRefCount::PostDestroySelf(cThread *Thread) const
{
	Thread->callback([this]() {DestroySelf(); }, eCallbackType::NoImmediate);
}
