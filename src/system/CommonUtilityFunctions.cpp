#include "StdAfx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


int DoesFileExist(const cPath &FileName)
{
	HANDLE FileHandle=CreateFile(FileName.c_str(),0,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(FileHandle==INVALID_HANDLE_VALUE)
		return false;
	CloseHandle(FileHandle);
	return true;
}

void TruncateFileAt(const std::string &FileName,__int64 NewFileSize)
{
	HANDLE FileHandle=CreateFile(FileName.c_str(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
	if(FileHandle==INVALID_HANDLE_VALUE)
		ThrowLastError(fmt::sprintf("CreateFile(\"%s\")",FileName));
	if(!SetFilePointerEx(FileHandle,*(LARGE_INTEGER *)&NewFileSize,NULL,FILE_BEGIN))
		ThrowLastError(fmt::sprintf("SetFilePointerEx(\"%s\")",FileName));
	if(!SetEndOfFile(FileHandle))
		ThrowLastError(fmt::sprintf("SetEndOfFile(\"%s\")",FileName));
	CloseHandle(FileHandle);
}

__int64 fileSize(const std::string &FileName)
{
	HANDLE FileHandle=CreateFile(FileName.c_str(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(FileHandle==INVALID_HANDLE_VALUE)
		ThrowLastError(fmt::sprintf("CreateFile(\"%s\")",FileName));
	__int64 FileSize;
	if(!GetFileSizeEx(FileHandle,(LARGE_INTEGER *)&FileSize))
		ThrowLastError(fmt::sprintf("GetFileSizeEx(\"%s\")",FileName));
	CloseHandle(FileHandle);
	return FileSize;
}

namespace
{
	bool IsWs(const char c)
	{
		return c==' '||c=='\t';
	}
}

void TrimRight(std::string &s)
{
	auto i=std::reversed_find_if_not(cend(s), cbegin(s), IsWs);
	if(i!=cend(s))
	{
		s.erase(i+1, cend(s));
	}
}

void TrimLeft(std::string &s)
{
	auto i=std::find_if_not(cbegin(s), cend(s), IsWs);
	s.erase(cbegin(s), i);
}

void Trim(std::string &s)
{
	TrimRight(s);
	TrimLeft(s);
}
