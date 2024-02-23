#pragma once

class cFont2
{
public:
	struct cLetterData
	{
		tIntrusivePtr<cTexture> mTexture;
		int mAdvance;
		int mXOffset, mYOffset;
	};
private:
	friend class cFontManager2;
	std::string mName;
	tIntrusivePtr<cTexture> mAtlasTexture;
	std::vector<cLetterData> mNormalLetters; // array doesn't work because msvc bug (http://stackoverflow.com/questions/34052576/stdarray-of-a-private-nested-struct-cant-be-initialized-error-c2248)
	std::unordered_map<wchar_t, cLetterData> mExtraLetters;
	int mFontHeight=0;
	int mAscender=0;
	int mDescender=0;
public:
	cFont2(const std::string &Name)
		: mName(Name)
	{
		mNormalLetters.resize(128);
	}
	const std::string &name() const { return mName; }

	std::vector<std::unique_ptr<cSpriteBase>> CreateTextSprites(const UTF8::cDecodedString &Text, const cRect &Rect, eHorizontalAlign HorizontalAlign, eVerticalAlign VerticalAlign) const;
	std::vector<std::unique_ptr<cSpriteBase>> CreateTextSprites(const UTF8::cDecodedString &Text);
	tIntrusivePtr<cTexture> CreateTexture(const UTF8::cDecodedString &Text) const;
	struct cTextMetrics
	{
		cPoint mBoundingSize;
		std::vector<cPoint> mLineSizes;
	};
	cTextMetrics MeasureText(const UTF8::cDecodedString &Text) const;
	int GetHeight() const { return mFontHeight; } // temp
	int GetAdvance(int Char) const;
	int GetAscender() const { return mAscender; }
	int GetDescender() const { return mDescender; }
	const cLetterData &GetLetterData(int Char) const;
	const cLetterData &GetLetterData(wchar_t Char) const;

	tIntrusivePtr<cTexture> Debug_GetAtlasTexture() const { return mAtlasTexture; }
};
