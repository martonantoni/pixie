#pragma once

#pragma pack(push, 1)

struct cRiffHeader
{
    uint32_t id;
    uint32_t mBlockSize;
    uint32_t format;
};

struct cRiffRecordHeader
{
    uint32_t signature;
    uint32_t length;
};

#pragma pack(pop)

class cRiffFile
{
    cFile mFile;
    cRiffHeader mHeader;
    struct cRiffRecord
    {
        uint32_t mSignature;
        uint32_t mLength;
        uint64_t mPosition;
    };
    std::vector<cRiffRecord> mRecords;
    bool mWasError = false;
public:
    cRiffFile(const cPath& path);
    uint32_t format() const
    {
        return mHeader.format;
    }
    bool operator!() const { return mWasError; }
    operator bool() const { return !mWasError; }
    std::vector<char> ReadChunkData(uint32_t chunkId);
};
