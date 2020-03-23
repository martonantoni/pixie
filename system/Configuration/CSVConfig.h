#pragma once

class cCSVConfig
{
	cStringVector mKeys;
	typedef cStringVector cRow;
	typedef std::vector<cRow> cRows;
	cRows mRows;
public:
	cCSVConfig(const cPath &Path, const std::string &Separator=",");

	size_t GetNumberOfRows() const { return mRows.size(); }

	int GetInt(size_t RowIndex, size_t KeyIndex) const;
	double GetDouble(size_t RowIndex, size_t KeyIndex) const;
	std::string GetString(size_t RowIndex, size_t KeyIndex) const;
	bool GetBool(size_t RowIndex, size_t KeyIndex) const;
	bool IsFieldEmpty(size_t RowIndex, size_t KeyIndex) const;

	size_t GetKeyIndex(const std::string &Key) const; // return ~0 if no such key
};
