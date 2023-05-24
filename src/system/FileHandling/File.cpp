#include "StdAfx.h"

cFile::cFile(cFile &&Other)
	: mFileHandle(Other.mFileHandle)
	, mWasError(Other.mWasError)
{
	Other.mFileHandle=INVALID_HANDLE_VALUE;
}

cFile &cFile::operator=(cFile &&Other)
{
	if(&Other==this)
		return *this;
	if(mFileHandle!=INVALID_HANDLE_VALUE)
		::CloseHandle(mFileHandle);
	mFileHandle=Other.mFileHandle;
	mWasError=Other.mWasError;
	Other.mFileHandle=INVALID_HANDLE_VALUE;
	return *this;
}

cFile::~cFile()
{
	if(mFileHandle!=INVALID_HANDLE_VALUE)
	{
		::CloseHandle(mFileHandle);
	}
}

bool cFile::Open(const cPath &Path, unsigned int OpenFlags)
{
	if(mWasError)
		return false;
	if(mFileHandle!=INVALID_HANDLE_VALUE)
		::CloseHandle(mFileHandle);
	unsigned int DesiredAccess=0, ShareMode=0;
	if(OpenFlags&Open_Read)
	{
		DesiredAccess|=GENERIC_READ;
		ShareMode|=FILE_SHARE_READ|FILE_SHARE_WRITE;
	}
	if(OpenFlags&Open_Write)
	{
		DesiredAccess|=GENERIC_WRITE;
		ShareMode|=FILE_SHARE_READ;
	}
	unsigned int CreationDisposition=OPEN_EXISTING;
	if(OpenFlags&Open_Create)
	{
		CreationDisposition=CREATE_ALWAYS;
	}
	if(OpenFlags&Open_Truncate)
	{
		CreationDisposition=CREATE_ALWAYS;
	}
	mFileHandle=::CreateFile(Path.c_str(), DesiredAccess, ShareMode, NULL, CreationDisposition, NULL, NULL);
	int Err=GetLastError();
	mWasError=mFileHandle==INVALID_HANDLE_VALUE;
	return mWasError;
}

uint64_t cFile::GetPosition() const
{
	if(mWasError)
		return 0;
	LARGE_INTEGER NewPosition, CurrentPosition;
	NewPosition.QuadPart=0i64;
	if(!::SetFilePointerEx(mFileHandle, NewPosition, &CurrentPosition, SEEK_CUR))
	{
		mWasError=true;
		return 0;
	}
	return CurrentPosition.QuadPart;
}

void cFile::SetPosition(uint64_t NewPosition, eSeekType SeekType)
{
	if(mWasError)
		return;
	LARGE_INTEGER NewPositionLI;
	NewPositionLI.QuadPart=NewPosition;
	unsigned int SeekMethod=SEEK_SET;
	switch(SeekType)
	{
	case Seek_Set:
		SeekMethod=SEEK_SET;
		break;
	case Seek_Cur:
		SeekMethod=SEEK_CUR;
		break;
	case Seek_End:
		SeekMethod=SEEK_END;
		break;
	}
	if(!::SetFilePointerEx(mFileHandle, NewPositionLI, NULL, SeekMethod))
	{
		mWasError=true;
		return;
	}
}

uint64_t cFile::GetSize() const
{
	if(mWasError)
		return 0;
	LARGE_INTEGER Size;
	if(!::GetFileSizeEx(mFileHandle, &Size))
	{
		mWasError=true;
		return 0;
	}
	return Size.QuadPart;
}


uint64_t cFile::Read(char *Destination, uint64_t MaxBytesToRead)
{
	if(mWasError)
		return 0;
	DWORD NumberOfBytesRead=0;
	if(!::ReadFile(mFileHandle, Destination, (DWORD)MaxBytesToRead, &NumberOfBytesRead, NULL))
	{
		mWasError=true;
		return 0;
	}
	return NumberOfBytesRead;
}

void cFile::Write(const char *Source, uint64_t BytesToWrite)
{
	if(mWasError)
		return;
	DWORD NumberOfBytesWritten=0;
	if(!::WriteFile(mFileHandle, Source, (DWORD)BytesToWrite, &NumberOfBytesWritten, NULL))
	{
		mWasError=true;
		return;
	}
}

bool cFile::WasError() const
{
	return mWasError;
}

void cFile::ClearError()
{
	mWasError=false;
}

std::vector<uint8_t> FileHandling::ReadFile(cFile &File)
{
	std::vector<uint8_t> FileData;
	FileData.resize(File.GetSize());
	File.SetPosition(0);
	File.Read((char *)FileData.data(), FileData.size());
	return FileData;
}

std::tuple<std::vector<uint8_t>, bool> FileHandling::ReadFile(const cPath &Path)
{
	cFile File;
	File.Open(Path, cFile::Open_Read);
	auto FileData=FileHandling::ReadFile(File);
	return std::tuple<std::vector<uint8_t>, bool> { FileData, !File.WasError() };
}

bool FileHandling::WriteFile(const cPath &Path, const std::vector<uint8_t> &Data)
{
	cFile File;
	File.Open(Path, cFile::Open_Truncate|cFile::Open_Write|cFile::Open_Create);
	File.Write((const char *)Data.data(), Data.size());
	return File.WasError();
}
