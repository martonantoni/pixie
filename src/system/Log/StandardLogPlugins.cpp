#include "StdAfx.h"

#include "LogChunk.h"
#include "StandardLogPlugins.h"

#include <iostream>

using namespace StandardLogPlugins;

void StandardLogPlugins::cEcho::Open()
{
	// Empty function
}

void StandardLogPlugins::cEcho::Close()
{
	// Empty function
}

void StandardLogPlugins::cEcho::LogChunkPart(cLog::cChunkPart *ChunkPart)
{
	std::cout.write(ChunkPart->Chunk.GetData()+ChunkPart->Offset,ChunkPart->Length);
	delete ChunkPart;
}

void StandardLogPlugins::cEcho::Flush()
{
	std::cout.flush();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void StandardLogPlugins::cFile::Open()
{
	FileHandle=CreateFile(AttachedLog->GetFileName().c_str(),GENERIC_WRITE,FILE_SHARE_READ,NULL,
		AttachedLog->GetFlags()&cLog::TRUNCATE?CREATE_ALWAYS:OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(FileHandle==INVALID_HANDLE_VALUE)
	{
		ThrowLastError(fmt::sprintf("CreateFile(\"%s\")",AttachedLog->GetFileName()));
	}
	if(!(AttachedLog->GetFlags()&cLog::NO_AUTO_SEEK_END))
	{
		LARGE_INTEGER EndPosition;
		EndPosition.QuadPart=0;
		if(!SetFilePointerEx(FileHandle,EndPosition,NULL,SEEK_END))
			ThrowLastError("SetFilePointerEx");
	}
}

void StandardLogPlugins::cFile::Close()
{
	if(FileHandle!=INVALID_HANDLE_VALUE)
		CloseHandle(FileHandle);
}

void StandardLogPlugins::cFile::LogChunk(cLog::cChunk *Chunk)
{
	if(FileHandle!=INVALID_HANDLE_VALUE)
	{
		DWORD Written;
		if(!WriteFile(FileHandle,Chunk->GetData(),Chunk->GetOffset(),&Written,NULL))
			ThrowLastError("WriteFile");
	}
	delete Chunk;
}

void StandardLogPlugins::cFile::Flush()
{
	if(FileHandle!=INVALID_HANDLE_VALUE&&!FlushFileBuffers(FileHandle))
		ThrowLastError("FlushFileBuffers");
}

void StandardLogPlugins::cFile::Seek(cSeekParameters *SeekPars)
{
	if(FileHandle!=INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER NewPosition;
		NewPosition.QuadPart=SeekPars->first;
		if(!SetFilePointerEx(FileHandle,NewPosition,NULL,SeekPars->second))
			ThrowLastError("SetFilePointerEx");
	}
}

void StandardLogPlugins::cFile::Tell(__int64 *Position)
{
	if(FileHandle!=INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER NewPosition,CurrentPosition;
		NewPosition.QuadPart=0i64;
		if(!SetFilePointerEx(FileHandle,NewPosition,&CurrentPosition,SEEK_CUR))
			ThrowLastError("SetFilePointerEx");
		*Position=CurrentPosition.QuadPart;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// void cDebugFile::Open()
// {
// 	FileHandle=CreateFile(AttachedLog->GetFileName(), GENERIC_WRITE, FILE_SHARE_READ, NULL,
// 		AttachedLog->GetFlags()&cLog::TRUNCATE?CREATE_ALWAYS:OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
// 	if(FileHandle==INVALID_HANDLE_VALUE)
// 	{
// 		ThrowLastError(fmt::sprintf("CreateFile(\"%s\")", AttachedLog->GetFileName()));
// 	}
// 	if(!(AttachedLog->GetFlags()&cLog::NO_AUTO_SEEK_END))
// 	{
// 		LARGE_INTEGER EndPosition;
// 		EndPosition.QuadPart=0;
// 		if(!SetFilePointerEx(FileHandle, EndPosition, NULL, SEEK_END))
// 			ThrowLastError("SetFilePointerEx");
// 	}
// }
// 
// void cDebugFile::Close()
// {
// 	// Empty function
// }
// 
// void cDebugFile::LogChunkPart(cLog::cChunkPart *ChunkPart)
// {
// 	WriteFile(mFileHandle, ChunkPart->Chunk.GetData()+ChunkPart->Offset, ChunkPart->Length, NULL, NULL);
// 	delete ChunkPart;
// }
// 
// void cDebugFile::Flush()
// {
// }
