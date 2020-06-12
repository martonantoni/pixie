#include "StdAfx.h"

cSoundPlayer theSoundPlayer;

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

namespace
{

    HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
    {
        HRESULT hr = S_OK;
        if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
            return HRESULT_FROM_WIN32(GetLastError());

        DWORD dwChunkType;
        DWORD dwChunkDataSize;
        DWORD dwRIFFDataSize = 0;
        DWORD dwFileType;
        DWORD bytesRead = 0;
        DWORD dwOffset = 0;

        while (hr == S_OK)
        {
            DWORD dwRead;
            if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
                hr = HRESULT_FROM_WIN32(GetLastError());

            if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
                hr = HRESULT_FROM_WIN32(GetLastError());

            switch (dwChunkType)
            {
            case fourccRIFF:
                dwRIFFDataSize = dwChunkDataSize;
                dwChunkDataSize = 4;
                if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
                    hr = HRESULT_FROM_WIN32(GetLastError());
                break;

            default:
                if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
                    return HRESULT_FROM_WIN32(GetLastError());
            }

            dwOffset += sizeof(DWORD) * 2;

            if (dwChunkType == fourcc)
            {
                dwChunkSize = dwChunkDataSize;
                dwChunkDataPosition = dwOffset;
                return S_OK;
            }

            dwOffset += dwChunkDataSize;

            if (bytesRead >= dwRIFFDataSize)
                return S_FALSE;

        }
        return S_OK;
    }

    HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
    {
        HRESULT hr = S_OK;
        if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
            return HRESULT_FROM_WIN32(GetLastError());
        DWORD dwRead;
        if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

}

void cSoundPlayer::Initialize()
{
    HRESULT hr;
    if (FAILED(hr = XAudio2Create(&mXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
    {
        RELEASE_ASSERT_EXT(false, "XAudio2Create failed");
    }

    if (FAILED(hr =mXAudio2->CreateMasteringVoice(&mMasterVoice)))
    {
        RELEASE_ASSERT_EXT(false, "CreateMasteringVoice failed");
    }
}

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


void cSoundPlayer::Play(const char* filename)
{
    cRiffFile file(filename);
    
    RELEASE_ASSERT(file);
    auto wfxChunkData = file.ReadChunkData('fmt ');
    auto dataChunkData = file.ReadChunkData('data');
    if (!wfxChunkData.empty() && !dataChunkData.empty())
    {
        XAUDIO2_BUFFER buffer = { 0 };
        buffer.AudioBytes = dataChunkData.size();  //buffer containing audio data
        buffer.pAudioData = (const BYTE*)dataChunkData.data();  //size of the audio buffer in bytes
        buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

        IXAudio2SourceVoice* pSourceVoice;
        if (FAILED(mXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)wfxChunkData.data()))) 
            return;

        if (FAILED(pSourceVoice->SubmitSourceBuffer(&buffer)))
            return;

        if (FAILED(pSourceVoice->Start(0)))
            return;

        mEffects.emplace_back();
        mEffects.back().mData = std::move(dataChunkData);
    }
}
