#pragma once

#include "Log.h"

/*
Note:
	When the data is modified (using LogArgs, and LogBinary), there should be no other threads using the same object.
	Sharing it between threads is only safe when all of those threads are only reading the Chunk.
*/

class cLog::cChunk
{
	struct cData
	{
		__declspec(align(4)) volatile long ReferenceCounter;
		char *Data;
		int Length;
		cData(int pLength);
		~cData();
	};
	cData *SharedData;
	int Offset;
public:
	cChunk();
	cChunk(int Length);
	cChunk(const cChunk &Source);
	const cChunk &operator=(const cChunk &pSource);
	~cChunk();

	void Release();
	const char *GetData() const;
	int GetLength() const;
	int GetOffset() const { return Offset; }
//////////
	void ResetOffset();
	int LogArgs(const char *FormatString,va_list Args,int Flags); // returns written length, -1 on error
	int LogBinary(const char *Data,int Length);			// returns written length, -1 on error
};

struct cLog::cChunkPart
{
	cLog::cChunk Chunk;
	int Offset;
	int Length;
	cChunkPart(cLog::cChunk *pChunk,int pOffset,int pLength);
};

