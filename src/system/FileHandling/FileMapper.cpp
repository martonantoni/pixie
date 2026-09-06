#include "StdAfx.h"


cFileMapper::cFileMapper(const std::string &pFileName): FileName(pFileName)
{
	FileHandle=::CreateFile(pFileName.c_str(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if(FileHandle==INVALID_HANDLE_VALUE)
		ThrowLastError(std::format("CreateFile(\"{}\")",pFileName));
	FileMappingHandle=::CreateFileMapping(FileHandle,NULL,PAGE_WRITECOPY,0,0,NULL);
	if(FileMappingHandle==INVALID_HANDLE_VALUE)
		ThrowLastError(std::format("CreateFileMapping(\"{}\")",pFileName));
	GetFileSizeEx(FileHandle,(LARGE_INTEGER *)&FileSize);
	SYSTEM_INFO SystemInfo;
	::GetSystemInfo(&SystemInfo);
	SystemGranuality=SystemInfo.dwAllocationGranularity;

	ViewOffset=0;
	ViewPosition=NULL;
	VirtualPosition=0;

	if(FileSize)
		MoveView(1);
}

cFileMapper::~cFileMapper()
{
	if(FileHandle!=INVALID_HANDLE_VALUE)
	{
		if(ViewPosition)
			if(!::UnmapViewOfFile(ViewPosition))
				ThrowLastError(std::format("UnmapViewOfFile failed. File: {}",FileName));
		if(FileMappingHandle!=INVALID_HANDLE_VALUE)
			CloseHandle(FileMappingHandle);
		CloseHandle(FileHandle);
	}
}

void cFileMapper::MoveView(unsigned MinimumViewSize)
{
	if(ViewPosition)
		if(!::UnmapViewOfFile(ViewPosition))
			ThrowLastError(std::format("UnmapViewOfFile failed. File: {}",FileName));

	__int64 AlignError=VirtualPosition%SystemGranuality;
	__int64 AlignedVirtualPosition=VirtualPosition-AlignError;
	MinimumViewSize+=Low32(AlignError);

	ViewPosition=(char *)::MapViewOfFile(FileMappingHandle,FILE_MAP_READ,High32(AlignedVirtualPosition),Low32(AlignedVirtualPosition),MinimumViewSize);
	if(!ViewPosition)
		ThrowLastError(std::format("MapViewOfFile failed (offset= {}, size: {}). File: {}",(int)ViewOffset,(int)ViewSize,FileName));

	MEMORY_BASIC_INFORMATION MemoryInfo;
	::VirtualQuery(ViewPosition,&MemoryInfo,sizeof(MEMORY_BASIC_INFORMATION));
	int AllocationOffset=(int)((char *)MemoryInfo.BaseAddress-(char *)MemoryInfo.AllocationBase);
	ViewSize=(unsigned)MemoryInfo.RegionSize+AllocationOffset;
	ViewPosition=(const char *)MemoryInfo.AllocationBase;
	ViewOffset=AlignedVirtualPosition-AllocationOffset;

//	MainLog->Log("*MV* AllocOffset={}, ViewSize={}, ViewOffset={}, AllocBase=0x{:x}", AllocationOffset, ViewSize, (int)ViewOffset, (int)ViewPosition);
}

const char *cFileMapper::GetView(unsigned Length,int MovePosition)
{
	if(VirtualPosition+Length>FileSize)
		return NULL;
	if(VirtualPosition<ViewOffset||VirtualPosition+Length>=ViewOffset+ViewSize)
		MoveView(Length);
	const char *RequestedPosition=ViewPosition+(VirtualPosition-ViewOffset);
	if(MovePosition)
		VirtualPosition+=Length;
	return RequestedPosition;
}

_int64 cFileMapper::Seek(__int64 NewLocation,int MoveMethod)
{
	switch(MoveMethod)
	{
	case SEEK_CUR:
		VirtualPosition+=NewLocation;
		break;
	case SEEK_END:
		VirtualPosition=FileSize-NewLocation;
		break;
	case SEEK_SET:
		VirtualPosition=NewLocation;
		break;
	default:
		THROW_DETAILED_EXCEPTION(std::format("Unknown seek method ({})",MoveMethod));
	}
	return VirtualPosition;
}

void cFileMapper::Read(void *Destination,int Length)
{
	const char *RequestedView=GetView(Length,true);
	memcpy((char *)Destination,RequestedView,Length);
}

//////////////////////////////////////////////////////////////////////////
/*
void TestFileMapper()
{
	{
		cFileMapper File("eq2idb.dat");
		cLog TestLog("copy.dat",cLog::TRUNCATE);
		for(;;)
		{
			const char *MyView=File.GetView(1000);
			if(!MyView)
				break;
			TestLog.LogBinary(MyView,1000);
		}
		MainLog->Log("Copy done.\n");
	}
	TruncateFileAt("copy.dat",1995);
}*/