#pragma once

class cPath
{
	std::string mPath;
	struct cParsedData
	{
		bool mIsValid=false;
		std::string mFileNameWithoutExtension;
		std::string mExtension;
		cStringVector mPathParts;
	};
	mutable cParsedData mParsedData;
	void UpdateParsedDataIfNeeded() const;
public:
	cPath()=default;
	cPath(const char *Path): mPath(Path) {}
	cPath(const std::string &Path): mPath(Path) {}
	cPath(const cPath &)=default;
	cPath &operator=(const cPath &)=default;

	std::string GetFileName() const;
	std::string GetExtension() const;
	std::string GetFileNameWithoutExtension() const;
	std::string ToString() const { return mPath; }
	const char *c_str() const { return mPath.c_str(); }
	cPath operator+(const cPath &Path) const;
	bool empty() const { return mPath.empty(); }

	static std::vector<cPath> GetAllFilePath(const cPath &Folder, const std::string &Filter);
};

