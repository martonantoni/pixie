#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "RiffFile.hpp"

cRiffFile::cRiffFile(const cPath& path)
{
    mFile.Open(path, cFile::Open_Read);
    mFile.Read(reinterpret_cast<char*>(&mHeader), sizeof(mHeader));
    mHeader.id = ntohl(mHeader.id);
    mHeader.format = ntohl(mHeader.format);
    if (mHeader.id != 'RIFF')
    {
        MainLog->Log("file id is not RIFF: %s", path.c_str());
        mWasError = true;
        return;
    }
    for(;;)
    {
        cRiffRecordHeader recordHeader;
        uint64_t bytesRead = mFile.Read(reinterpret_cast<char*>(&recordHeader), sizeof(recordHeader));
        if (!bytesRead)
            break;
        MainLog->Log("Found chunk: %.4s", (const char*)&recordHeader.signature);
        mRecords.emplace_back();
        mRecords.back().mLength = recordHeader.length;
        mRecords.back().mSignature = ntohl(recordHeader.signature);
        mRecords.back().mPosition = mFile.GetPosition();
        mFile.SetPosition((mRecords.back().mLength + 3)&~3, cFile::Seek_Cur);
    }
    mWasError = mFile.WasError();
}

std::vector<char> cRiffFile::ReadChunkData(uint32_t chunkId) 
{
    auto i = std::find_if(ALL(mRecords), [chunkId](auto& record) { return record.mSignature == chunkId; });
    if (i == mRecords.end())
        return {};
    mFile.SetPosition(i->mPosition, cFile::Seek_Set);
    std::vector<char> data(i->mLength);
    mFile.Read(data.data(), i->mLength);
    return data;
}

