#pragma once

class cFileMapper
{
	std::string FileName; // Stored for debug purposes
	enum { MaxViewSize = 0x20000 }; 
	HANDLE FileHandle,FileMappingHandle;
	int SystemGranuality; // Size of memory pages
	__int64 FileSize;
	__int64 ViewOffset;
	const char *ViewPosition;
	unsigned int ViewSize;
	__int64 VirtualPosition;
	void MoveView(unsigned MinimumViewSize);
public:
	cFileMapper(const std::string &FileName);
	~cFileMapper();

	__int64 GetFileSize() const { return FileSize; }
	__int64 Tell() const { return VirtualPosition; }
	__int64 Seek(__int64 NewLocation,int MoveMethod);
	const char *GetView(unsigned int Length,int MovePosition=true); // returns NULL if there is not enough bytes left in the file

	// Backward compatibility:
	void Read(void *Destination,int Length);
    template<class T> void Read(T &Destination)  { Read(&Destination,sizeof(T)); }
	const std::string &GetFileName() const { return FileName; }
};

void TestFileMapper();