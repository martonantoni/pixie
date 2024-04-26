#pragma once

namespace Pixie
{

struct cTextRenderer2Config
{
    struct
    {
        std::shared_ptr<const cFont> mRegular;
        std::shared_ptr<const cFont> mBold;
        std::shared_ptr<const cFont> mItalic;
        std::shared_ptr<const cFont> mBoldItalic;
        std::vector<std::shared_ptr<const cFont>> mHeadings;
        std::shared_ptr<const cFont> mMonospace;
    } mFonts;
    struct
    {
        cColor mDefaultColor;
        cColor mLinkRegular;
        cColor mLinkHover;
        cColor mCodeBlockBG;
        cColor mCodeBlockBorder;
    } mColors;
    int mTabWidth;              // in pixels
    std::vector<int> mTabStops; // might be empty, use mTabWidth in that case (and after the last tab stop)
    int mWidth;                 // in pixels

// for sprite creation:
    cPixieWindow* mWindow = nullptr;
};

struct cTextRenderer2Span
{
    std::string_view mText;
    bool mIsBold = false;
    bool mIsItalic = false;
    bool mIsLink = false;
    bool mIsMonospace = false;
    union
    {
        int mLinkIndex = 0;
        int mCodeBlockIndex;
    };
};

struct cTextRenderer2Block
{
    std::vector<cTextRenderer2Span> mSpans;
    enum class eAlign { Left, Center, Right, Justify } mAlign = eAlign::Left;
    bool mIsCodeBlock = false;
    int mHeadingLevel = -1; // -1 means not a heading
};

struct cTextRenderer2BlockResult
{
    std::vector<std::unique_ptr<cSpriteBase>> mSprites;
    int mHeight = 0;
    struct cLinkInfo
    {
        int mFirstSpriteIndex;
        int mLastSpriteIndex;
        int mLinkIndex;
        cRect mRect;
    };
    std::vector<cLinkInfo> mLinks;
};

class cTextRenderer2
{
    cTextRenderer2Config mConfig;
    struct cWord
    {
        int mFirstSpriteIndex;
        int mLastSpriteIndex;
        int mStartX; // inclusive
        int mEndX;   // exclusive
        int mHeight;
        int mAscender;
        int width() const { return mEndX - mStartX; }
    };
    std::vector<cWord> mWords;
    std::pair<const cFont&, const cColor&> determineFont(const cTextRenderer2Block& block, const cTextRenderer2Span& span) const;
    using eAlign = cTextRenderer2Block::eAlign;
    using cSpan = cTextRenderer2Span;
    using cBlock = cTextRenderer2Block;
    cTextRenderer2BlockResult renderCodeBlock(const cBlock& block);
    int arrangeWords(
        eAlign align,
        std::vector<std::unique_ptr<cSpriteBase>>& sprites,
        int lineFirstWordIndex,
        int lineLastWordIndex,
        int lineYOffset) const;
public:
    void init(const cTextRenderer2Config& config);


    cTextRenderer2BlockResult render(const cTextRenderer2Block& block);


    /*

    @hx heading, x: number 1-9
    @| paragraph centered
    @< paragraph left aligned
    @> paragraph right aligned
    @= paragraph justified
    @h1 @| centered heading 1
    alma **bold text** korte __italic text__ barack
    alma [[link text]] korte
    __**bold italic**__
    ``monospace text``
    [[__italic link__]] [[**bold link**]] [[__**bold italic link**__]]

    @img texture_name
    @img_scaled texture_name

    ```
    code 
    block
    ```

    */
    static std::vector<cTextRenderer2Block> parse(const std::string& text);
};


}
