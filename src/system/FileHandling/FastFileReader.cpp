#include "StdAfx.h"


cFastFileReader::cFastFileReader(const std::filesystem::path& path) :
    mPath(path)
{
    mFileHandle = ::CreateFile(path.string().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (mFileHandle == INVALID_HANDLE_VALUE)
    {
        throwLastError(std::format("CreateFile(\"{}\")", mPath.string()));
    }
    mFileMappingHandle = ::CreateFileMapping(mFileHandle, NULL, PAGE_WRITECOPY, 0, 0, NULL);
    if (mFileMappingHandle == INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(mFileHandle);
        throwLastError(std::format("CreateFileMapping(\"{}\")", mPath.string()));
    }
    GetFileSizeEx(mFileHandle, (LARGE_INTEGER*)&mFileSize);
    SYSTEM_INFO SystemInfo;
    ::GetSystemInfo(&SystemInfo);
    mSystemGranuality = SystemInfo.dwAllocationGranularity;

    mViewOffset = 0;
    mViewPosition = mPosition = mEndPosition = NULL;
}

cFastFileReader::~cFastFileReader()
{
    if (mFileHandle != INVALID_HANDLE_VALUE)
    {
        if (mViewPosition)
            if (!::UnmapViewOfFile(mViewPosition))
                throwLastError(std::format("UnmapViewOfFile failed. File: {}", mPath.string()));
        if (mFileMappingHandle != INVALID_HANDLE_VALUE)
            CloseHandle(mFileMappingHandle);
        CloseHandle(mFileHandle);
    }
}

int cFastFileReader::moveView()
{
    if (mViewPosition)
        if (!::UnmapViewOfFile(mViewPosition))
            throwLastError(std::format("UnmapViewOfFile failed. File: {}", mPath.string()));
    __int64 Offset = (mPosition - mViewPosition) + mViewOffset;
    if (Offset == mFileSize)
    {
        mViewPosition = NULL;
        return false;
    }
    __int64 OffsetError = Offset % mSystemGranuality;
    mViewOffset = Offset - OffsetError;
    int ViewSize = Low32(std::min<__int64>(mFileSize - mViewOffset, MaxViewSize));
    mViewPosition = (char*)::MapViewOfFile(mFileMappingHandle, FILE_MAP_COPY, High32(mViewOffset), Low32(mViewOffset), ViewSize);
    if (!mViewPosition)
        throwLastError(std::format("MapViewOfFile failed (offset= {}, size: {}). File: {}", mViewOffset, ViewSize, mPath.string()));
    mPosition = mViewPosition + OffsetError;
    mEndPosition = mViewPosition + ViewSize;
    return true;
}

// 0x0a
// 0x0d 0x0a

std::pair<cFastFileReader::cLine, bool> cFastFileReader::getNextLine()
{
    if (mPosition == mEndPosition)
    {
        if (!moveView())
            return { {}, true };
    }
    // Find the end of the line
    for (;;) // (try again if there was not enough buffer first)
    {
        char* lineEnd = mPosition;
        for (; lineEnd != mEndPosition; ++lineEnd)
        {
            if (*lineEnd <= 0xd)
            {
                if (*lineEnd == 0xa || *lineEnd == 0)
                    break;
            }
        }
        if (lineEnd == mEndPosition)
        { // Did not find the line's end
            __int64 OldViewOffset = mViewOffset;
            if (!moveView())
                return { {}, true };
            if (mViewOffset == OldViewOffset)
            {
                if (mViewOffset + (mEndPosition - mViewPosition) == mFileSize)
                {
                    --lineEnd;
                    char* LineStart = mPosition;
                    mPosition = mEndPosition;
                    return { cLine(LineStart, (int)(lineEnd - LineStart + 1)) , false };
                }
                else
                    throw std::runtime_error(std::format("Too long line in file (ViewOffset: {}). File: {}", mViewOffset, mPath.string()));
            }
        }
        else
        {
            char* lineStart = mPosition;
            mPosition = lineEnd + 1; // start of the next line
            int lineLength = static_cast<int>(lineEnd - lineStart);
            if (lineLength >= 1 && lineStart[lineLength - 1] == 0xd)
                --lineLength;
            return { cLine(lineStart, lineLength), false };
        }
    }
    return { {}, true };
}

cFastFileReader::iterator cFastFileReader::begin()
{
    auto [firstLine, isEOF] = getNextLine();
    if (isEOF)
        return {};
    return iterator(*this, firstLine);
}
