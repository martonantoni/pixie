#include "StdAfx.h"

void cPath::UpdateParsedDataIfNeeded() const
{
	if(mParsedData.mIsValid)
		return;
	mParsedData.mIsValid=true;
	mParsedData.mPathParts.FromString(mPath, "/", false);
	auto &FileName=mParsedData.mPathParts[mParsedData.mPathParts.size()-1];
	auto DotPos=FileName.find_last_of('.');
	if(DotPos==std::string::npos)
	{
		mParsedData.mExtension.clear();
		mParsedData.mFileNameWithoutExtension=FileName;
	}
	else
	{
		mParsedData.mExtension=FileName.substr(DotPos+1);
		mParsedData.mFileNameWithoutExtension=FileName.substr(0, DotPos);
	}
}

std::string cPath::GetFileName() const
{
	UpdateParsedDataIfNeeded();
	return mParsedData.mPathParts[mParsedData.mPathParts.size()-1];
}

std::string cPath::GetExtension() const
{
	UpdateParsedDataIfNeeded();
	return mParsedData.mExtension;
}

std::string cPath::GetFileNameWithoutExtension() const
{
	UpdateParsedDataIfNeeded();
	return mParsedData.mFileNameWithoutExtension;
}

cPath cPath::operator+(const cPath &Path) const
{
	if(mPath.empty())
		return Path;
	if(Path.mPath.empty())
		return *this;
	return cPath(mPath+"/"+Path.mPath);
}

std::vector<cPath> cPath::GetAllFilePath(const cPath &Folder, const std::string &Filter)
{
	std::vector<cPath> Result;

	WIN32_FIND_DATA FindData;
	memset(&FindData, 0, sizeof(WIN32_FIND_DATA));
	HANDLE FindHandle=::FindFirstFile((Folder+Filter).c_str(), &FindData);
	if(FindHandle!=INVALID_HANDLE_VALUE)
	{
		do
		{
			if(!(FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
			{
				Result.emplace_back(Folder+std::string(FindData.cFileName));
			}
		} while(::FindNextFile(FindHandle, &FindData));
		::FindClose(FindHandle);
	}
	return Result;
}
