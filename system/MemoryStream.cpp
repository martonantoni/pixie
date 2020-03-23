#include "StdAfx.h"

cMemoryStream::cMemoryStream(cMemoryStream &&Source)
{
	MoveCommon(std::move(Source));
}

cMemoryStream &cMemoryStream::operator=(cMemoryStream &&Source)
{
	free(mData);
	MoveCommon(std::move(Source));
	return *this;
}

void cMemoryStream::MoveCommon(cMemoryStream &&Source)
{
	mData=Source.mData;
	mLength=Source.mLength;
	mAllocatedLength=Source.mAllocatedLength;
	mPosition=Source.mPosition;
	mAllocationBlockSize=Source.mAllocationBlockSize;
	Source.mData=nullptr;
	Source.mLength=Source.mAllocatedLength=0;
	Source.mPosition=0;
}

cMemoryStream::~cMemoryStream()
{
	free(mData);
}

void cMemoryStream::SetMinimumAllocatedLength(size_t MinimumAllocatedLength)
{
	if(MinimumAllocatedLength<=mAllocatedLength)
		return;
	mAllocatedLength=(MinimumAllocatedLength+mAllocationBlockSize-1)&~(mAllocationBlockSize-1);
	mData=(BYTE *)realloc(mData, mAllocatedLength);
}

void cMemoryStream::SetMinimumLength(size_t MinimumLength)
{
	if(MinimumLength<=mLength)
		return;
	SetMinimumAllocatedLength(MinimumLength);
	mLength=MinimumLength;
}

OWNERSHIP char *cMemoryStream::ReleaseAllData()
{
	char *Data=(char *)mData;
	mData=nullptr;
	return Data;
}

void cMemoryStream::SetPosition(size_t Position) const
{
	mPosition=Position;
}

size_t cMemoryStream::GetPosition() const
{
	return mPosition;
}

void cMemoryStream::Truncate()
{
	SetMinimumAllocatedLength(mPosition);
	mLength=mPosition;
}

void cMemoryStream::ShrinkToFit()
{
	mAllocatedLength=(mLength+mAllocationBlockSize-1)&~(mAllocationBlockSize-1);
	mData=(BYTE *)realloc(mData, mAllocatedLength);
}

void cMemoryStream::Reserve(size_t Size)
{
	SetMinimumAllocatedLength(mPosition+Size);
}

void cMemoryStream::WriteString(const std::string &Text)
{
	WriteData(Text.c_str(), Text.length()+1);
}

std::string cMemoryStream::ReadString() const
{
	if(mReadErrorHappened||ASSERTFALSE(!memchr(mData+mPosition, 0, GetLengthLeft())))
	{
		mReadErrorHappened=true;
		return std::string();
	}
	std::string StringRead((const char *)mData+mPosition);
	AdvancePosition(StringRead.length()+1);
	return StringRead;
}

void cMemoryStream::WriteQWORD(QWORD Value)
{
	SetMinimumLength(sizeof(QWORD)+mPosition);
	mData[mPosition]=Value&0xff;
	mData[mPosition+1]=(Value>>8)&0xff;
	mData[mPosition+2]=(Value>>16)&0xff;
	mData[mPosition+3]=(Value>>24)&0xff;
	mData[mPosition+4]=(Value>>32)&0xff;
	mData[mPosition+5]=(Value>>40)&0xff;
	mData[mPosition+6]=(Value>>48)&0xff;
	mData[mPosition+7]=(Value>>56)&0xff;
	AdvancePosition(sizeof(QWORD));
}

QWORD cMemoryStream::ReadQWORD() const
{
	if(mReadErrorHappened||ASSERTFALSE(GetLengthLeft()<sizeof(QWORD)))
	{
		mReadErrorHappened=true;
		return 0;
	}
	QWORD Value=QWORD(mData[mPosition])|(QWORD(mData[mPosition+1])<<8)|(QWORD(mData[mPosition+2])<<16)|(QWORD(mData[mPosition+3])<<24)|
		(QWORD(mData[mPosition+4])<<32)|(QWORD(mData[mPosition+5])<<40)|(QWORD(mData[mPosition+6])<<48)|(QWORD(mData[mPosition+7])<<56);
	AdvancePosition(sizeof(QWORD));
	return Value;
}

void cMemoryStream::WriteDWORD(DWORD Value)
{
	SetMinimumLength(sizeof(DWORD)+mPosition);
	mData[mPosition]=Value&0xff;
	mData[mPosition+1]=(Value>>8)&0xff;
	mData[mPosition+2]=(Value>>16)&0xff;
	mData[mPosition+3]=(Value>>24)&0xff;
	AdvancePosition(sizeof(DWORD));
}

DWORD cMemoryStream::ReadDWORD() const
{
	if(mReadErrorHappened||ASSERTFALSE(GetLengthLeft()<sizeof(DWORD)))
	{
		mReadErrorHappened=true;
		return 0;
	}
	DWORD Value=DWORD(mData[mPosition])|(DWORD(mData[mPosition+1])<<8)|(DWORD(mData[mPosition+2])<<16)|(DWORD(mData[mPosition+3])<<24);
	AdvancePosition(sizeof(DWORD));
	return Value;
}

void cMemoryStream::WriteWORD(WORD Value)
{
	SetMinimumLength(sizeof(WORD)+mPosition);
	mData[mPosition]=Value&0xff;
	mData[mPosition+1]=(Value>>8)&0xff;
	AdvancePosition(sizeof(WORD));
}

WORD cMemoryStream::ReadWORD() const
{
	if(ASSERTFALSE(GetLengthLeft()<sizeof(WORD)))
		return 0;
	WORD Value=WORD(mData[mPosition])|(WORD(mData[mPosition+1])<<8);
	AdvancePosition(sizeof(WORD));
	return Value;
}

void cMemoryStream::WriteBYTE(BYTE Value)
{
	SetMinimumLength(sizeof(BYTE)+mPosition);
	mData[mPosition]=Value;
	AdvancePosition(sizeof(BYTE));
}

BYTE cMemoryStream::ReadBYTE() const
{
	if(mReadErrorHappened||ASSERTFALSE(GetLengthLeft()<sizeof(BYTE)))
	{
		mReadErrorHappened=true;
		return 0;
	}
	BYTE Value=mData[mPosition];
	AdvancePosition(sizeof(BYTE));
	return Value;
}


void cMemoryStream::WriteData(const char *Data, size_t Length)
{
	SetMinimumLength(Length+mPosition);
	memcpy(mData+mPosition, Data, Length);
	AdvancePosition(Length);
}

bool cMemoryStream::ReadData(char *Data, size_t Length) const
{
	if(mReadErrorHappened||GetLengthLeft()<Length)
	{
		mReadErrorHappened=true;
		return false;
	}
	memcpy(Data, mData+mPosition, Length);
	AdvancePosition(Length);
	return true;
}

void cMemoryStream::WriteData(const std::vector<uint8_t> &Data)
{
	WriteDWORD((uint32_t)Data.size());
	WriteData((const char *)Data.data(), Data.size());
}

std::tuple<std::vector<uint8_t>, bool> cMemoryStream::ReadData() const
{
	auto Size=ReadDWORD();
	std::vector<uint8_t> Data;
	Data.resize(Size);
	auto Success=ReadData((char *)Data.data(), Size);
	return std::tuple<std::vector<uint8_t>, bool> { std::move(Data), Success };
}

void cMemoryStream::SetAllocationBlockSize(size_t AllocationBlockSize)
{
	ASSERT(IsPowerOfTwo(AllocationBlockSize));
	mAllocationBlockSize=AllocationBlockSize;
}

char *cMemoryStream::AccessData(size_t MinimumLengthNeeded)
{
	Reserve(MinimumLengthNeeded);
	return (char *)(mData+mPosition);
}

void cMemoryStream::ReportWritten(size_t Length)
{
	ASSERT(GetPosition()+Length<=mAllocatedLength);
	SetMinimumLength(GetPosition()+Length);
}

void cMemoryStream::SetAsStart()
{
	size_t LengthLeft=GetLengthLeft();
	if(LengthLeft)
		memmove(mData, mData+mPosition, LengthLeft);
	mPosition=0;
	mLength=LengthLeft;
}

void cMemoryStream::WriteStream(const cMemoryStream &SourceStream)
{
	WriteDWORD((DWORD)SourceStream.GetLength());
	WriteData(SourceStream.GetAllData(), SourceStream.GetLength());
}

cMemoryStream cMemoryStream::ReadStream() const
{
	cMemoryStream Stream;
	if(mReadErrorHappened)
		return Stream;
	DWORD Length=ReadDWORD();
	if(!Length)
		return Stream;
	if(ASSERTFALSE(Length<GetLengthLeft()))
	{
		mReadErrorHappened=true;
		return Stream;
	}
	Stream.WriteData((const char *)(mData+mPosition), Length);
	mPosition+=Length;
	return Stream;
}

