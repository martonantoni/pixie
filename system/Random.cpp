#include "StdAfx.h"

class cRandomKeeper: public tSingleton<cRandomKeeper>
{
	HCRYPTPROV hProvider;

	std::vector<unsigned char> mBuffer;
	size_t mOffset;
	void FillBuffer();
public:
	cRandomKeeper();
	~cRandomKeeper();
	unsigned char *GetBytes(size_t NumberOfBytes);
};

cRandomKeeper::cRandomKeeper()
{
	hProvider=0;
	if (!::CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
	{
		ASSERT(false);
		return;
	}
	mBuffer.resize(100);
	FillBuffer();
}

cRandomKeeper::~cRandomKeeper()
{
	::CryptReleaseContext(hProvider, 0);
}

void cRandomKeeper::FillBuffer()
{
	mOffset=0;
	if (!::CryptGenRandom(hProvider, (DWORD)mBuffer.size(), &mBuffer[0]))
	{
		ASSERT(false);
	}
}

unsigned char *cRandomKeeper::GetBytes(size_t NumberOfBytes)
{
	if(mOffset+NumberOfBytes>=mBuffer.size())
		FillBuffer();
	unsigned char *RetVal=&mBuffer[mOffset];
	mOffset+=(NumberOfBytes+3)&~3;
	return RetVal;
}

uint64_t GenerateRandomInteger64()
{
	return *(uint64_t *)cRandomKeeper::Get().GetBytes(sizeof(uint64_t));
}