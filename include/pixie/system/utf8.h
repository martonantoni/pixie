#pragma once

namespace UTF8
{

	std::vector<wchar_t> Decode(const std::string &Source); // returned vector will include the terminating 0 (and it will be terminted there)

	std::string Encode(const wchar_t *Source, size_t SourceLength); // SourceLength does not count the terminating 0 (and does not need to be present)

	inline bool DoesNeedDecode(const std::string &Text)
	{
		return std::none_of(ALL(Text), [](auto c) { return c&0x80; });
	}

	wchar_t DecodeCharacter(const char *&i);

	class cDecodedString
	{
		std::vector<wchar_t> mDecoded;
	public:
		cDecodedString(const std::string &EncodedString);
		auto begin() const { return mDecoded.begin(); }
		auto end() const { return mDecoded.end(); }
		auto operator[](size_t i) const { return mDecoded[i]; }
		auto size() const { return mDecoded.size()-1; } // do not count the terminating zero
		auto empty() const { return mDecoded.size()<=1; }
	};
}

inline auto cbegin(const UTF8::cDecodedString &DecodedString)
{
	return DecodedString.begin();
}

inline auto cend(const UTF8::cDecodedString &DecodedString)
{
	return DecodedString.end();
}