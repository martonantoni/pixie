#pragma once

class cFile
{
	mutable bool mWasError=false;
	HANDLE mFileHandle=INVALID_HANDLE_VALUE;
public:
	cFile()=default;
	cFile(const cFile &)=delete;
	cFile(cFile &&);
	cFile &operator=(const cFile &)=delete;
	cFile &operator=(cFile &&);
	~cFile();
	enum
	{
		Open_Read     = 1,
		Open_Write    = 2,
		Open_Create   = 4,
		Open_Truncate = 8,
	};
	bool Open(const cPath &Path, unsigned int OpenFlags);

	uint64_t GetPosition() const;
	enum eSeekType { Seek_Set, Seek_Cur, Seek_End };
	void SetPosition(uint64_t NewPosition, eSeekType SeekType=Seek_Set);
	uint64_t GetSize() const;

	uint64_t Read(char *Destination, uint64_t MaxBytesToRead);
	void Write(const char *Source, uint64_t BytesToWrite);
	bool WasError() const;
	void ClearError();
};

namespace FileHandling
{
	std::vector<uint8_t> ReadFile(cFile &File);
	std::tuple<std::vector<uint8_t>, bool> ReadFile(const cPath &Path);
	bool WriteFile(const cPath &Path, const std::vector<uint8_t> &Data);
	bool WriteFile(const cPath &Path, const void *Data, size_t Size);
}