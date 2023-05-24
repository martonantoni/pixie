#include "StdAfx.h"


cFastFileReader::cFastFileReader(const cPath &Path): FileName(Path.ToString())
{
	FileHandle=::CreateFile(FileName.c_str(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if(FileHandle==INVALID_HANDLE_VALUE)
		ThrowLastError(fmt::sprintf("CreateFile(\"%s\")",FileName.c_str()));
	FileMappingHandle=::CreateFileMapping(FileHandle,NULL,PAGE_WRITECOPY,0,0,NULL);
	if(FileMappingHandle==INVALID_HANDLE_VALUE)
	{
		::CloseHandle(FileHandle);
		ThrowLastError(fmt::sprintf("CreateFileMapping(\"%s\")", FileName.c_str()));
	}
	GetFileSizeEx(FileHandle,(LARGE_INTEGER *)&FileSize);
	SYSTEM_INFO SystemInfo;
	::GetSystemInfo(&SystemInfo);
	SystemGranuality=SystemInfo.dwAllocationGranularity;

	ViewOffset=0;
	ViewPosition=Position=EndPosition=NULL;
}

cFastFileReader::~cFastFileReader()
{
	if(FileHandle!=INVALID_HANDLE_VALUE)
	{
		if(ViewPosition)
			if(!::UnmapViewOfFile(ViewPosition))
				ThrowLastError(fmt::sprintf("UnmapViewOfFile failed. File: %s",FileName.c_str()));
		if(FileMappingHandle!=INVALID_HANDLE_VALUE)
			CloseHandle(FileMappingHandle);
		CloseHandle(FileHandle);
	}
}

int cFastFileReader::MoveView()
{ 
	if(ViewPosition)
		if(!::UnmapViewOfFile(ViewPosition))
			ThrowLastError(fmt::sprintf("UnmapViewOfFile failed. File: %s",FileName.c_str()));
	__int64 Offset=(Position-ViewPosition)+ViewOffset;
	if(Offset==FileSize)
	{
		ViewPosition=NULL;
		return false;
	}
	__int64 OffsetError=Offset%SystemGranuality;
	ViewOffset=Offset-OffsetError;
	int ViewSize=Low32(Min<__int64>(FileSize-ViewOffset,MaxViewSize));
	ViewPosition=(char *)::MapViewOfFile(FileMappingHandle,FILE_MAP_COPY,High32(ViewOffset),Low32(ViewOffset),ViewSize);
	if(!ViewPosition)
		ThrowLastError(fmt::sprintf("MapViewOfFile failed (offset= %d, size: %d). File: %s",ViewOffset,ViewSize,FileName.c_str()));
	Position=ViewPosition+OffsetError;
	EndPosition=ViewPosition+ViewSize;
	return true;
}

cFastFileReader::cLine cFastFileReader::GetNextLine()
{
// Find the real start of the line, skipping all line separators
	for(;;)
	{	
		for(;Position!=EndPosition;++Position)
		{
			if(*Position>0xd)
				break;
			if(*Position!=0xd&&*Position!=0xa&&*Position!=0)
				break;
		}
		if(Position==EndPosition)
		{
			if(!MoveView())
				return cLine(NULL,0);
		}
		else
			break;
	}
// Find the end of the line
	for(;;) // (try again if there was not enough buffer first)
	{
		char *LineEnd=Position;
		for(;LineEnd!=EndPosition;++LineEnd)
		{
			if(*LineEnd<=0xd)
			{
				if(*LineEnd==0xd||*LineEnd==0xa||*LineEnd==0)
				{
					break;
				}
			}
		}
		if(LineEnd==EndPosition)
		{ // Did not find the line's end
			__int64 OldViewOffset=ViewOffset;
			if(!MoveView())
				return cLine(NULL,0); // End of file
			if(ViewOffset==OldViewOffset)
			{
				if(ViewOffset+(EndPosition-ViewPosition)==FileSize)
				{
					--LineEnd;
					char *LineStart=Position;
					Position=EndPosition;
					return cLine(LineStart,(int)(LineEnd-LineStart+1));
				}
				else
					THROW_DETAILED_EXCEPTION(fmt::sprintf("Too long line in file (ViewOffset: %d). File: %s",ViewOffset,FileName.c_str()));
			}

		}
		else
		{
			char *LineStart=Position;
			Position=LineEnd;
			return cLine(LineStart,(int)(LineEnd-LineStart));
		}
	}
	return cLine(NULL,0);
}
