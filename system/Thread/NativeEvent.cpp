cNativeEvent::cNativeEvent(eType Type)
{
	mHandle=::CreateEvent(NULL,Type==ManualReset,false,NULL);
}

cNativeEvent::cNativeEvent(HANDLE Handle)
: mHandle(Handle)
{
}

cNativeEvent::~cNativeEvent()
{
	::CloseHandle(mHandle);
}

void cNativeEvent::Set()
{
	::SetEvent(mHandle);
}

void cNativeEvent::Reset()
{
	::ResetEvent(mHandle);
}

BOOL cNativeEvent::Wait(DWORD TimeOut)
{
	return ::WaitForSingleObject(mHandle,TimeOut)==WAIT_OBJECT_0;
}
