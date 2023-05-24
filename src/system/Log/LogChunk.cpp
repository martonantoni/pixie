#include "StdAfx.h"

#include "LogChunk.h"

#include <time.h>

cLog::cChunk::cChunk(): SharedData(NULL), Offset(0)
{
}

cLog::cChunk::cChunk(const cChunk &Source): Offset(Source.Offset)
{
	SharedData=Source.SharedData;
	if(SharedData)
	{
		_InterlockedIncrement(&SharedData->ReferenceCounter);
	}
}

cLog::cChunk::cChunk(int Length): Offset(0)
{
	SharedData=new cData(Length);
}

cLog::cChunk::cData::cData(int pLength)
{
	ReferenceCounter=1;
	Length=pLength;
	Data=new char[pLength+1];
}

cLog::cChunk::cData::~cData()
{
	ASSERT(ReferenceCounter==0);
	delete[] Data;
}

cLog::cChunk::~cChunk()
{
	Release();
}

void cLog::cChunk::Release()
{
	ASSERT(SharedData);
	if(!_InterlockedDecrement(&SharedData->ReferenceCounter))
	{
		delete SharedData;
		SharedData=NULL;
	}
}

const cLog::cChunk &cLog::cChunk::operator=(const cChunk &Source)
{
	if(Source.SharedData==SharedData)
		return *this;
	Release();
	SharedData=Source.SharedData;
	if(SharedData)
	{
		_InterlockedIncrement(&SharedData->ReferenceCounter);
	}
	return *this;
}

const char *cLog::cChunk::GetData() const
{
	return SharedData?SharedData->Data:NULL;
}

int cLog::cChunk::GetLength() const
{
	return SharedData?SharedData->Length:0;
}

void cLog::cChunk::ResetOffset()
{
	Offset=0;
}
 
int cLog::cChunk::LogArgs(const char *FormatString,va_list Args,int Flags)
{
	if(!SharedData)
		return -1;
	int HeaderLength=(Flags&cLog::TIME_STAMP)?9:0;
	int FullOffset=Offset+HeaderLength;
	int BackPaddingLength=(Flags&cLog::NO_LINE_FEED)?0:2;
	if(SharedData->Length-FullOffset-BackPaddingLength<=0)
		return -1;
	int Written=vsnprintf_s(SharedData->Data+FullOffset,SharedData->Length-FullOffset+1,
		SharedData->Length-FullOffset-BackPaddingLength,FormatString,Args);
	if(Written==-1)
		return -1;
	if(Flags&cLog::TIME_STAMP)
	{
		time_t CurrentTime=time(0);
		tm LocalTime;
		localtime_s(&LocalTime,&CurrentTime);
		char *TimeStampPos=SharedData->Data+Offset;
		TimeStampPos[0]='0'+LocalTime.tm_hour/10;
		TimeStampPos[1]='0'+LocalTime.tm_hour%10;
		TimeStampPos[2]=':';
		TimeStampPos[3]='0'+LocalTime.tm_min/10;
		TimeStampPos[4]='0'+LocalTime.tm_min%10;
		TimeStampPos[5]=':';
		TimeStampPos[6]='0'+LocalTime.tm_sec/10;
		TimeStampPos[7]='0'+LocalTime.tm_sec%10;
		TimeStampPos[8]=' ';
	}
	if(!(Flags&cLog::NO_LINE_FEED))
	{
		SharedData->Data[FullOffset+Written]=0xd;
		SharedData->Data[FullOffset+Written+1]=0xa;
	}
	Offset=FullOffset+Written+BackPaddingLength;
	return Written+HeaderLength+BackPaddingLength;
}

int cLog::cChunk::LogBinary(const char *Data,int Length)
{
	if(!SharedData||SharedData->Length-Offset<Length)
		return -1;
	memcpy(SharedData->Data+Offset,Data,Length);
	Offset+=Length;
	return Length;
}

cLog::cChunkPart::cChunkPart(cLog::cChunk *pChunk,int pOffset,int pLength):
	Chunk(*pChunk), Length(pLength), Offset(pOffset)
{
}