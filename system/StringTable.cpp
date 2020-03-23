#include "StdAfx.h"

#include "StringTable.h"

cStringTable::cStringTable()
{
	ThreadDataIndex=::TlsAlloc();
	if(ThreadDataIndex==TLS_OUT_OF_INDEXES)
		ThrowLastError("TlsAlloc");
	RootData=new cThreadData;
}

cStringTable::cThreadData::cThreadData()
{
	HashTable.resize(0x100);
}

const char *cStringTable::GetUniqueTextInstance(const CString &String)
{
	int HashValue=0;
	for(const char *Pos=String;*Pos;++Pos)
		HashValue+=*Pos;
	HashValue&=0xff;
	cThreadData *ThreadData=(cThreadData *)::TlsGetValue(ThreadDataIndex);
	if(!ThreadData)
	{
		ThreadData=new cThreadData;
		::TlsSetValue(ThreadDataIndex,ThreadData);
	}
	const char *&ThreadLocalUniqueText=ThreadData->HashTable[HashValue][String];
	if(ThreadLocalUniqueText)
		return ThreadLocalUniqueText;

	cMutexGuard Guard(RootMutex);
	const char *&RootUniqueText=RootData->HashTable[HashValue][String];
	if(!RootUniqueText)
	{
		RootUniqueText=new char[String.GetLength()+1];
		__movsb((PBYTE)RootUniqueText,(const PBYTE)(const char *)String,String.GetLength()+1);
	}
	ThreadLocalUniqueText=RootUniqueText;
	return ThreadLocalUniqueText;
}