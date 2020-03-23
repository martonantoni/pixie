
cThreadLocalStorage::cThreadLocalStorage()
{
	mThreadDataIndex=::TlsAlloc();
	RELEASE_ASSERT(mThreadDataIndex!=TLS_OUT_OF_INDEXES);
}

cThreadLocalStorage::~cThreadLocalStorage()
{
	::TlsFree(mThreadDataIndex);
}

void *cThreadLocalStorage::GetValue() const
{
	return ::TlsGetValue(mThreadDataIndex);
}

void cThreadLocalStorage::SetValue(void *NewValue) const
{
	::TlsSetValue(mThreadDataIndex,NewValue);
}
