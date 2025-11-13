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
    int mTabWidth = 2;          // in spaces
};

struct cTextRenderer2Target
{
    cPixieWindow* mWindow = nullptr;
    int mWidth = std::numeric_limits<int>::max();      // in pixel
    std::vector<int> mTabStops; // in pixels! might be empty, use mTabWidth in that case (and after the last tab stop)
};

struct cTextRenderer2Span
{
    std::string_view mText;
    std::span<const cColor> mColors; // for each character in mText, if empty, use the default color
    std::optional<cColor> mOverrideColor; // if set, use this color for the entire span
    char mSeparator = 0;
    bool mIsBold = false;
    bool mIsItalic = false;
    bool mIsLink = false;
    bool mIsMonospace = false;
    int mCodeBlockIndex;
    std::string_view mLinkId;
};

struct cTextRenderer2Block
{
    std::vector<cTextRenderer2Span> mSpans;
    enum class eAlign { Left, Center, Right, Justify } mAlign = eAlign::Left;
    bool mIsCodeBlock = false;
    bool mIsListItem = false;
    bool mEmptyLineAfter = true;
    struct
    {
        std::string mTitle;
        std::string mType;
        int mFixedWidth = -1; // -1 means not set, otherwise in characters
                              // if target width is smaller, use that
    } mCodeBlock;
    int mHeadingLevel = -1; // -1 means not a heading
    int mTabWidth = -1; // -1 means not set (use config)
};

struct cTextRenderer2BlockResult
{
    std::vector<std::unique_ptr<cSpriteBase>> mSprites;
    int mHeight = 0;
    struct cLinkInfo
    {
        int mFirstSpriteIndex;
        int mLastSpriteIndex;
        std::string mId; // either the link text or the explicit id
        cRect mRect;
    };
    std::vector<cLinkInfo> mLinks;
};

using cTextColorSelector = std::variant<
    std::monostate,
    std::function<std::optional<cColor>(const cTextRenderer2Span&)>, 
    std::function<std::optional<cColor>(const cTextRenderer2Span&, std::string_view)>,
    std::function<std::optional<cColor>(std::string_view)>>;

class cTextRenderer2
{
    cTextRenderer2Config mConfig;
    cTextRenderer2Target mTarget;
    cTextColorSelector mColorSelector;
    struct cWord final
    {
        int mFirstSpriteIndex;
        int mLastSpriteIndex;
        int mStartX; // inclusive
        int mEndX;   // exclusive
        int mHeight;
        int mAscender;
        int mWidth;
        int mSpaceWidth;
        char separator = 0;
        int width() const { return mWidth; }
    };
    class cPrefixRemover final
    {
        std::string_view& mText;
        int mPrefixSize;
    public:
        cPrefixRemover(std::string_view& text, int prefixSize) : mText(text), mPrefixSize(prefixSize) {}
        ~cPrefixRemover();
        void overridePrefixSize(int size);
    };
    std::vector<cWord> mWords;
    std::pair<const cFont&, cColor> determineFont(const cTextRenderer2Block& block, const cTextRenderer2Span& span) const;
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
    int getNextTabStop(int x, int tabWidthOverride) const;
public:
    void init(const cTextRenderer2Config& config, const cTextRenderer2Target& target);
    void setTarget(const cTextRenderer2Target& target);
    void setTargetWidth(int width);
    const cTextRenderer2Config& config() const { return mConfig; }

    template<class T> void setColorSelector(T&& colorSelector) requires std::constructible_from<cTextColorSelector, T>
    {
        // Used during the render call
        mColorSelector = std::forward<T>(colorSelector);
    }

    cTextRenderer2BlockResult render(const cTextRenderer2Block& block);
    static std::unique_ptr<cMultiSpriteBase> quickRender(const std::string& font, const std::string& text);


    /*

    -- : comment (entire line is ignored)
    @hx heading, x: number 1-9
    @| paragraph centered, @|+ default align: centered
    @< paragraph left aligned, @<+ default align: left aligned
    @> paragraph right aligned, @>+ default align: right aligned
    @= paragraph justified, @=+ default align: justified
    @txx override tab width for paragraph, xx: number of spaces
    @h1 @| centered heading 1
    alma **bold text** korte __italic text__ barack
    alma [[link text]] korte [[link_id:link text]] barack
    __**bold italic**__
    ``monospace text``
    [[__italic link__]] [[**bold link**]] [[__**bold italic link**__]]

    @img texture_name
    @img_scaled texture_name

    @c color_name : set color
    @cd : back to default color

    ```type:title:width       all 3 are optional. For example setting 20 width: "```::20"
    code 
    block
    ```

    - list item
    - list item
    - list item

    */
    static std::vector<cTextRenderer2Block> parse(const std::string& text);
};


}
