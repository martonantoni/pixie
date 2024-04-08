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
	template<class T> T get(size_t RowIndex, size_t KeyIndex) const;
	bool IsFieldEmpty(size_t RowIndex, size_t KeyIndex) const;

	size_t GetKeyIndex(const std::string &Key) const; // return ~0 if no such key
};

template<class T> T cCSVConfig::get(size_t RowIndex, size_t KeyIndex) const
{
	if constexpr (std::is_same_v<T, int>)
		return GetInt(RowIndex, KeyIndex);
	else if constexpr (std::is_same_v<T, double>)
		return GetDouble(RowIndex, KeyIndex);
	else if constexpr (std::is_same_v<T, std::string>)
		return GetString(RowIndex, KeyIndex);
	else if constexpr (std::is_same_v<T, bool>)
		return GetBool(RowIndex, KeyIndex);
	else
		ASSERT(false);
	return {};
}
