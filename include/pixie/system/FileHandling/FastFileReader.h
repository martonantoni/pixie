#pragma once

// Supports both CRLF and LF line endings, but not CR (mac old style).
// Windows only, uses memory mapped files for performance.

#ifndef _WIN32
#error "cFastFileReader is only supported on Windows"
#endif

class cFastFileReader final
{
public:
	using cLine = std::string_view;
    class iterator
    {
    public:
        // --- Iterator traits ---
        using iterator_category = std::forward_iterator_tag;
        using value_type = cLine;
        using difference_type = std::ptrdiff_t;
        using reference = cLine;
        using pointer = const cLine*;

    private:
        cFastFileReader* mReader = nullptr;
        cLine mLine{};
        bool mIsEnd = true;

    public:
        iterator() = default; // end iterator
        iterator(cFastFileReader& reader, cLine firstLine);
        reference operator*() const { return mLine; }
        pointer operator->() const { return &mLine; }
        iterator& operator++();
        iterator operator++(int);
        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;
    };
private:
    const std::filesystem::path mPath; // Stored for debug purposes
	static constexpr int MaxViewSize = 0x20000; 
	HANDLE mFileHandle, mFileMappingHandle;
	int mSystemGranuality; // Size of memory pages
	__int64 mFileSize;
	__int64 mViewOffset;
	char *mViewPosition;
	char *mPosition;
	char *mEndPosition;
	int moveView(); // returns true if there is more of the file to read
public:
	cFastFileReader(const std::filesystem::path &Path);
	~cFastFileReader();

    std::pair<cLine, bool> getNextLine(); // returns line, isEOF
	__int64 fileSize() const { return mFileSize; }
	const std::filesystem::path& path() const { return mPath; }
    iterator begin();
    iterator end() { return {}; }
};


inline cFastFileReader::iterator::iterator(cFastFileReader& reader, cLine firstLine)
    : mReader(&reader), mLine(firstLine), mIsEnd(false)
{
}

inline cFastFileReader::iterator& cFastFileReader::iterator::operator++()
{
    if (!mIsEnd)
    {
        std::tie(mLine, mIsEnd) = mReader->getNextLine();
    }
    return *this;
}

inline cFastFileReader::iterator cFastFileReader::iterator::operator++(int)
{
    iterator tmp = *this;
    ++(*this);
    return tmp;
}

inline bool cFastFileReader::iterator::operator==(const iterator& other) const
{
    if (mIsEnd && other.mIsEnd)
        return true;
    if (mIsEnd != other.mIsEnd)
        return false;
    return mReader == other.mReader;
}

inline bool cFastFileReader::iterator::operator!=(const iterator& other) const
{
    return !(*this == other);
}
