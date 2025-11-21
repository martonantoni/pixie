#pragma once

class cFastFileReader
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
        iterator(cFastFileReader& reader, cLine firstLine)
            : mReader(&reader), mLine(firstLine), mIsEnd(false)
        {
        }
        reference operator*() const { return mLine; }
        pointer operator->() const { return &mLine; }
        iterator& operator++()
        {
            if (!mIsEnd) 
            {
                std::tie(mLine, mIsEnd) = mReader->GetNextLine();
            }
            return *this;
        }
        iterator operator++(int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        bool operator==(const iterator& other) const
        {
            if (mIsEnd && other.mIsEnd)
                return true;
            if (mIsEnd != other.mIsEnd)
                return false;
            return mReader == other.mReader;
        }
        bool operator!=(const iterator& other) const 
        {
            return !(*this == other);
        }
    };
private:
	std::string FileName; // Stored for debug purposes
	enum { MaxViewSize = 0x20000 }; 
	HANDLE FileHandle,FileMappingHandle;
	int SystemGranuality; // Size of memory pages
	__int64 mFileSize;
	__int64 mViewOffset;
	char *mViewPosition;
	char *mPosition;
	char *mEndPosition;
	int MoveView(); // returns true if there is more of the file to read
public:
	cFastFileReader(const cPath &Path);
	~cFastFileReader();

    std::pair<cLine, bool> GetNextLine(); // returns line, isEOF
	__int64 GetFileSize() const { return mFileSize; }
	const std::string &GetFileName() const { return FileName; }
    iterator begin();
    iterator end() { return {}; }
};