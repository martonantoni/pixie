#pragma once

class cMemoryStream: public tIntrusiveListItemBase<cMemoryStream>
{
	BYTE *mData=nullptr;
	size_t mAllocatedLength=0;
	size_t mLength=0;
	mutable size_t mPosition=0;
	size_t mAllocationBlockSize=0x1000;   // always power of 2
	mutable bool mReadErrorHappened=false;
	void SetMinimumAllocatedLength(size_t MinimumAllocatedLength);
	void SetMinimumLength(size_t MinimumLength);
	void MoveCommon(cMemoryStream &&Source);
public:
	cMemoryStream() {}
	cMemoryStream(const cMemoryStream &) = delete;
	cMemoryStream(cMemoryStream &&Source);
	cMemoryStream &operator=(const cMemoryStream &) = delete;
	cMemoryStream &operator=(cMemoryStream &&Source);
	virtual ~cMemoryStream();
	const char *GetAllData() const { return (const char *)mData; }
	size_t GetLength() const { return mLength; }
	OWNERSHIP char *ReleaseAllData();        // returned data needs to be freed with free()!

	// as a stream:
	void SetPosition(size_t Position) const;  // can move to virtual pos (outside current size), writing there fills the "gap"
	size_t GetPosition() const;               // might be larger than Length (see SetPosition)
	void AdvancePosition(size_t PositionOffset) const { SetPosition(GetPosition()+PositionOffset); }
	void MovePosition(__int64 PositionOffset) const { SetPosition(__int64(GetPosition())+PositionOffset); }
	size_t GetLengthLeft() const { return GetPosition()<GetLength()?GetLength()-GetPosition():0; }
	void Truncate();
	void Reserve(size_t Size);                // calculated from the current position
	void ShrinkToFit();

	void WriteString(const std::string &Text);
	std::string ReadString() const;

	bool WasReadError() const { return mReadErrorHappened; }

	void WriteQWORD(QWORD Value);
	QWORD ReadQWORD() const;
	void WriteDWORD(DWORD Value);
	DWORD ReadDWORD() const;
	void WriteWORD(WORD Value);
	WORD ReadWORD() const;
	void WriteBYTE(BYTE Value);
	BYTE ReadBYTE() const;

	void WriteBool(bool Value) { WriteBYTE(Value?1:0); }
	bool ReadBool() const { return ReadBYTE(); }

	cMemoryStream ReadStream() const;
	void WriteStream(const cMemoryStream &SourceStream);

	void WriteData(const char *Data, size_t Length);
	bool ReadData(char *Data, size_t Length) const;
	void WriteData(const std::vector<uint8_t> &Data);
	std::tuple<std::vector<uint8_t>, bool> ReadData() const;

	const char *GetData() const { return (const char *)(mData+mPosition); }
	
	char *AccessData(size_t MinimumLengthNeeded);
	void ReportWritten(size_t Length);
	
	void SetAsStart();

	// semi-internal (optimalization):
	void SetAllocationBlockSize(size_t AllocationBlockSize); // must be power of 2
};

template<class T> size_t CorrectTruncatedSZ(T Value) 
{
	return Value==std::numeric_limits<T>::max()?~0:size_t(Value); 
}