#pragma once

// control:
//
// {c:color_name}text text text{c}  : set color
// {f:font_name}text text text{f}   : set font
// {y:value}                        : modify the vertical position of text cursor by value (might be negative)
// {t:t0,t1,t2...}                  : set the tab positions. for example: {t:120,250} is the same as calling setTabStops(std::vector<int>{120,250});
// {>} {|} {=} : until the next new line: alignment right, center, justified
// {i:texture_name}  : insert texture into text
// {g:group_index}text text text{g} : for retrieving sprite indeces and bounding box in cRenderInfo
// \n: new paragraph
// \r: new paragraph, but no new line
//

class cTextRenderer
{
	cFont2 *mDefaultFont=nullptr;
	std::vector<int> mTabStops;
	cColor mDefaultColor;
	unsigned int mMaxWidth=~0u;

	struct cWord
	{
		enum class eType { Word, LineBreak, LineBreak_SameLine, Tab, Change_Y, Invalid } mType=eType::Invalid;
		std::vector<std::unique_ptr<cSprite>> mSprites;
		int mWidth;
		int mAscender=0;
		int mHeight=0;
		int mDescender=0;
		int mChangeValue=0;

		cWord(std::vector<std::unique_ptr<cSprite>> &&Sprites, int Width, const cFont2 &Font): 
			mSprites(std::move(Sprites)), mType(eType::Word), mWidth(Width), 
			mAscender(Font.GetAscender()), mHeight(Font.GetHeight()), mDescender(Font.GetDescender()) {}
		cWord(int Width): mType(eType::Word), mWidth(Width) {}
		cWord(eType Type): mType(Type) {}
		cWord(eType Type, int ChangeValue): mType(Type), mChangeValue(ChangeValue) { ASSERT(Type==eType::Change_Y); }
		cWord()=default;
		cWord(cWord &&)=default;
		cWord(const cWord &)=delete;
		cWord &operator=(const cWord &)=delete;
		cWord &operator=(cWord &&)=default;
	};
	typedef std::vector<cWord> cWords;
	struct cLine
	{
		cWords mWords;
		int mSumWidth=0;
		int mTabStopCounter=0;
		int mNextLineYOffset=0;
		cLine()=default;
		cLine(const cLine &)=delete;
		cLine(cLine &&)=default;
	};
	class cDocument
	{
	public:
		enum class eAlignment { Left, Right, Center, Justified };
	private:
		std::vector<int> mTabStops;
		eAlignment mParagraphAlignment=eAlignment::Left;
		const cTextRenderer &mParent;
		size_t mSpriteIndex=0;
		enum class eNewLineType { LineBreak, MaxWidthReached, SameY, EndOfDocument };
		void NewLine(eNewLineType Type);
	public:
		std::vector<cLine> mLines;
		std::vector<cColor> mColorStack;
		std::vector<cFont2 *> mFontStack;
		std::vector<int> mGroupStack;

		cDocument(const cTextRenderer &Parent): 
			mParent(Parent), mColorStack { Parent.mDefaultColor }, mFontStack { Parent.mDefaultFont }, mTabStops(Parent.mTabStops) { mLines.resize(1); }
		void AddWord(cWord &&NewWord);
		void SetAlignment(eAlignment Alignment);
		void EndOfDocument();
		size_t GetSpriteIndex() const;
		void setTabStops(std::vector<int> TabStops) { mTabStops=std::move(TabStops); }
	};
	static cWord CreateWordFromTexture(const std::string &TextureName);

public:
	cTextRenderer(cFont2 *DefaultFont=theFontManager.GetFont("default")): mDefaultFont(DefaultFont) {}
	cTextRenderer(const std::string &DefaultFont): mDefaultFont(theFontManager.GetFont(DefaultFont)) {}
	cTextRenderer(const std::string& defaultFont, const std::string& defaultColor): mDefaultFont(theFontManager.GetFont(defaultFont)), mDefaultColor(theColorServer.GetColor(defaultColor)) {}
	void setFont(cFont2 *defaultFont) { mDefaultFont=defaultFont; }
	struct cRenderInfo
	{
		int mNextLineY;
		struct cGroupInfo
		{
			size_t mFirstSpriteIndex=0, mLastSpriteIndex=0;
		};
		std::vector<cGroupInfo> mGroups;
	};
	std::vector<std::unique_ptr<cSpriteBase>> render(const std::string &Text, OUT cRenderInfo *RenderInfo=nullptr) const;

	void setTabStops(std::vector<int> TabStops) // pixel pos from start of line
	{
		mTabStops=std::move(TabStops);
	}
	void setDefaultColor(cColor Color)
	{
		mDefaultColor=Color;
	}
	void setMaxWidth(unsigned int MaxWidth)
	{
		mMaxWidth=MaxWidth;
	}
	cFont2& defaultFont() const { return *mDefaultFont; }
};