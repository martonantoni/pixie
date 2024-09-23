#include "StdAfx.h"

#include "pixie/pixie/2dEngine/text_renderer_2.h"

namespace Pixie
{

void cTextRenderer2::init(const cTextRenderer2Config& config, const cTextRenderer2Target& target)
{
    mConfig = config;
    mTarget = target;
}

void cTextRenderer2::setTarget(const cTextRenderer2Target& target)
{
    mTarget = target;
}

void cTextRenderer2::setTargetWidth(int width)
{
    mTarget.mWidth = width;
}

std::pair<const cFont&, cColor>  cTextRenderer2::determineFont(const cTextRenderer2Block& block, const cTextRenderer2Span& span) const
{
    auto& color = span.mIsLink ? mConfig.mColors.mLinkRegular : mConfig.mColors.mDefaultColor;
    if (block.mIsCodeBlock || span.mIsMonospace)
    {
        return { *mConfig.mFonts.mMonospace, color };
    }
    if(span.mIsBold)
    {
        if(span.mIsItalic)
        {
            return { *mConfig.mFonts.mBoldItalic, color };
        }
        return { *mConfig.mFonts.mBold, color };
    }
    if(span.mIsItalic)
    {
        return { *mConfig.mFonts.mItalic, color };
    }
    if(block.mHeadingLevel > 0)
    {
        if (block.mHeadingLevel < mConfig.mFonts.mHeadings.size())
        {
            return { *mConfig.mFonts.mHeadings[block.mHeadingLevel - 1], color };
        }
        return { *mConfig.mFonts.mBold, color };
    }
    return { *mConfig.mFonts.mRegular, color };
}

int cTextRenderer2::getNextTabStop(int x, int tabWidthOverride) const
{
    if (tabWidthOverride != -1)
    {
        for (int tabStop : mTarget.mTabStops)
        {
            if (tabStop > x)
            {
                return tabStop;
            }
        }
    }
    int tabWidthInSpaces = tabWidthOverride != -1 ? tabWidthOverride : mConfig.mTabWidth;
    int tabWidth = tabWidthInSpaces * mConfig.mFonts.mRegular->letterData(' ').advance();
    return x + tabWidth - x % tabWidth;
}

int cTextRenderer2::arrangeWords(
    cTextRenderer2Block::eAlign align,
    std::vector<std::unique_ptr<cSpriteBase>>& sprites,
    int lineFirstWordIndex, 
    int lineLastWordIndex,
    int lineYOffset) const
{
    int lineHeight = 0, ascender = 0;
    int wordsWidth = mWords[lineLastWordIndex].mEndX - mWords[lineFirstWordIndex].mStartX;
    for (int i = lineFirstWordIndex; i <= lineLastWordIndex; ++i)
    {
        lineHeight = std::max(lineHeight, mWords[i].mHeight);
        ascender = std::max(ascender, mWords[i].mAscender);
    }
    int lineXOffset = -mWords[lineFirstWordIndex].mStartX;
    switch (align)
    {
    case cTextRenderer2Block::eAlign::Left:
        for (int i = lineFirstWordIndex; i <= lineLastWordIndex; ++i)
        {
            int xOffset = mWords[i].mStartX;
            for (int j = mWords[i].mFirstSpriteIndex; j <= mWords[i].mLastSpriteIndex; ++j)
            {
                auto pos = sprites[j]->GetPosition();
                pos.y += lineYOffset + ascender - mWords[i].mAscender;
                pos.x += xOffset;
                sprites[j]->SetPosition(pos);
            }
        }
        break;
    case cTextRenderer2Block::eAlign::Center:
    {
        int centeringOffset = (mTarget.mWidth - wordsWidth) / 2;
        for (int i = lineFirstWordIndex; i <= lineLastWordIndex; ++i)
        {
            int xOffset = mWords[i].mStartX + centeringOffset;
            for (int j = mWords[i].mFirstSpriteIndex; j <= mWords[i].mLastSpriteIndex; ++j)
            {
                auto pos = sprites[j]->GetPosition();
                pos.y += lineYOffset + ascender - mWords[i].mAscender;
                pos.x += xOffset;
                sprites[j]->SetPosition(pos);
            }
        }
        break;
    }
    case cTextRenderer2Block::eAlign::Right:
    {
        int rightOffset = mTarget.mWidth - wordsWidth;
        for (int i = lineFirstWordIndex; i <= lineLastWordIndex; ++i)
        {
            int xOffset = mWords[i].mStartX + rightOffset;
            for (int j = mWords[i].mFirstSpriteIndex; j <= mWords[i].mLastSpriteIndex; ++j)
            {
                auto pos = sprites[j]->GetPosition();
                pos.y += lineYOffset + ascender - mWords[i].mAscender;
                pos.x += xOffset;
                sprites[j]->SetPosition(pos);
            }
        }
        break;
    }
    case cTextRenderer2Block::eAlign::Justify:
    {
        int spaceCount = lineLastWordIndex - lineFirstWordIndex;
        int missingSpace = mTarget.mWidth - wordsWidth;
        double extraSpace = spaceCount > 0 ? (double)missingSpace / (double)spaceCount : 0.0;
        double sum = 0.0;
        int extraSpaceToUse = 0;
        for (int i = lineFirstWordIndex; i <= lineLastWordIndex; ++i)
        {
            int xOffset = mWords[i].mStartX + extraSpaceToUse;
            for (int j = mWords[i].mFirstSpriteIndex; j <= mWords[i].mLastSpriteIndex; ++j)
            {
                auto pos = sprites[j]->GetPosition();
                pos.y += lineYOffset + ascender - mWords[i].mAscender;
                pos.x += xOffset;
                sprites[j]->SetPosition(pos);
            }
            sum += extraSpace;
            auto extraToUse = (int)sum;
            extraSpaceToUse += extraToUse;
            sum -= extraToUse;
        }
        break;
    }
    } // switch (block.mAlignment)
    return lineHeight;
}

cTextRenderer2BlockResult cTextRenderer2::render(const cTextRenderer2Block& block)
{
    if (block.mIsCodeBlock)
        return renderCodeBlock(block);

    mWords.clear();
    cTextRenderer2BlockResult result;
    auto& sprites = result.mSprites;
    bool isFirstSpan = true;

    for (auto& span : block.mSpans)
    {
        auto [font, color] = determineFont(block, span);
        if (mColorSelector.index() == 1)
        {
            color = std::get<1>(mColorSelector)(span).value_or(color);
        }
        int spaceWidth = font.letterData(' ').advance();
        auto addText = [&](std::string_view text)
            {
                while (!text.empty())
                {
                    auto wordEnd = text.find_first_of(" \t");
                    char separator = wordEnd == std::string_view::npos ? 0 : text[wordEnd];
                    std::string_view wordView = wordEnd == std::string_view::npos ? text : text.substr(0, wordEnd);
                    text.remove_prefix(std::min(wordView.size() + 1, text.size()));

                    std::string_view textWord(wordView.data(), wordView.size());
                    cWord& word = mWords.emplace_back();
                    word.separator = separator ? separator : span.mSeparator;
                    word.mFirstSpriteIndex = sprites.size();
                    word.mHeight = font.height();
                    word.mAscender = font.ascender();
                    word.mSpaceWidth = spaceWidth;
                    if (mColorSelector.index() == 2)
                    {
                        color = std::get<2>(mColorSelector)(span, textWord).value_or(color);
                    }
                    else if (mColorSelector.index() == 3)
                    {
                        color = std::get<3>(mColorSelector)(textWord).value_or(color);
                    }
                    cPoint position(0, 0);
                    while (!textWord.empty())
                    {                        
                        if(size_t characterIndex = textWord.data() - span.mText.data(); characterIndex < span.mColors.size())
                        {
                            color = span.mColors[characterIndex];
                        }
                        wchar_t decodedChar = UTF8::popCharacter(textWord);
                        auto& letterData = font.letterData(decodedChar);
                        auto sprite = std::make_unique<cSprite>();
                        sprite->SetTextureAndSize(letterData.mTexture);
                        sprite->SetWindow(mTarget.mWindow);
                        sprite->SetRGBColor(color);
                        sprite->SetPosition(position + letterData.offset());
                        sprites.emplace_back(std::move(sprite));
                        position.x += letterData.advance();
                    }
                    word.mLastSpriteIndex = sprites.size() - 1;
                    word.mWidth = position.x;
                }
            };
        if (isFirstSpan && block.mIsListItem)
        {
            isFirstSpan = false;
            unsigned char bulletPoint[] = { 0xE2, 0x80, 0xA2, ' ' };   // bullet point
            std::string_view bulletPointView(reinterpret_cast<const char*>(bulletPoint), 4);
            addText(bulletPointView);
        };
        int firstSpriteInSpan = sprites.size();
        addText(span.mText);
        if (span.mIsLink)
        {
            auto& linkInfo = result.mLinks.emplace_back();
            linkInfo.mFirstSpriteIndex = firstSpriteInSpan;
            linkInfo.mLastSpriteIndex = sprites.size() - 1;
            linkInfo.mId = span.mLinkId;  // string_view to string
        }
    }
    int x = 0, lineYOffset = 0;
    int lineFirstWordIndex = 0;
    for (auto [idx, word] : std::views::enumerate(mWords))
    {
        if(x + word.width() > mTarget.mWidth)
        {
            int lineLastWordIndex = lineFirstWordIndex == idx ? idx : idx - 1;
            int lineHeight = arrangeWords(block.mAlign, sprites, lineFirstWordIndex, lineLastWordIndex, lineYOffset);
            result.mHeight += lineHeight;
            lineYOffset += lineHeight;
            lineFirstWordIndex = lineLastWordIndex + 1;
            x = 0;
        }
        word.mStartX = x;
        x = word.mEndX = x + word.width();            
        if (word.separator == '\t')
        {
            x = getNextTabStop(x, block.mTabWidth);
        }
        else if (word.separator == ' ')
        {
            x += word.mSpaceWidth;
        }
    }
    if (lineFirstWordIndex < mWords.size())
    {
        lineYOffset += arrangeWords(
            block.mAlign == eAlign::Justify ? eAlign::Left : block.mAlign,
            sprites, lineFirstWordIndex, mWords.size() - 1, lineYOffset);
    }
    result.mHeight = lineYOffset;
    if(block.mEmptyLineAfter)
        result.mHeight += mConfig.mFonts.mRegular->height();

    return result;
}

cTextRenderer2BlockResult cTextRenderer2::renderCodeBlock(const cBlock& block)
{
    cTextRenderer2BlockResult result;
    int headerHeight = 0;
    if (!block.mCodeBlock.mTitle.empty())
    {
        // ---- header BG ----
        auto headerFont = *mConfig.mFonts.mHeadings[2];
        headerHeight = headerFont.height() * 3 / 2;
        auto headerBackgroundSprite = std::make_unique<cSprite>();
        headerBackgroundSprite->SetTexture("1pix");
        headerBackgroundSprite->SetZOrder(99);
        headerBackgroundSprite->SetWindow(mTarget.mWindow);
        headerBackgroundSprite->SetRGBColor("dark_gray");
        headerBackgroundSprite->SetPosition(cPoint(0, 0));
        headerBackgroundSprite->SetSize(cPoint(mTarget.mWidth, headerHeight));
        result.mSprites.emplace_back(std::move(headerBackgroundSprite));
        // ---- header text ----
        std::string_view headerText = block.mCodeBlock.mTitle;
        cPoint headerPosition(10, headerFont.height() / 4);
        while (!headerText.empty())
        {
            wchar_t decodedChar = UTF8::popCharacter(headerText);
            auto& letterData = headerFont.letterData(decodedChar);
            auto sprite = std::make_unique<cSprite>();
            sprite->SetTextureAndSize(letterData.mTexture);
            sprite->SetZOrder(100);
            sprite->SetWindow(mTarget.mWindow);
            sprite->SetRGBColor("white");
            sprite->SetPosition(headerPosition + letterData.offset());
            result.mSprites.emplace_back(std::move(sprite));
            headerPosition.x += letterData.advance();
        }
    }

    // ---- block ----
    ASSERT(block.mSpans.size() == 1);
    auto& span = block.mSpans[0];
    const cFont& font = mConfig.mFonts.mMonospace ? *mConfig.mFonts.mMonospace : *mConfig.mFonts.mRegular;
    int startX = font.letterData(' ').advance();
    cPoint position(startX, font.height() / 2 + headerHeight);
    for (auto lineView : span.mText | std::views::split('\n'))
    {
        std::string_view line(lineView.data(), lineView.size());
        while (!line.empty())
        {
            wchar_t decodedChar = UTF8::popCharacter(line);
            auto& letterData = font.letterData(decodedChar);
            auto sprite = std::make_unique<cSprite>();
            sprite->SetTextureAndSize(letterData.mTexture);
            sprite->SetZOrder(100);
            sprite->SetWindow(mTarget.mWindow);
            sprite->SetRGBColor(mConfig.mColors.mDefaultColor);
            sprite->SetPosition(position + letterData.offset());
            result.mSprites.emplace_back(std::move(sprite));
            position.x += letterData.advance();
        }
        position.y += font.height();
        position.x = startX;
    }
    position.y += font.height() / 2;
    // ---- background ----
    auto backgroundSprite = std::make_unique<cSprite>();
    backgroundSprite->SetTexture("1pix");
    backgroundSprite->SetZOrder(98);
    backgroundSprite->SetWindow(mTarget.mWindow);
    backgroundSprite->SetRGBColor(mConfig.mColors.mCodeBlockBG);
    backgroundSprite->SetPosition(cPoint(0, headerHeight));
    backgroundSprite->SetSize(cPoint(mTarget.mWidth, position.y - headerHeight));
    result.mSprites.emplace_back(std::move(backgroundSprite));
    // ---- border  ----
    auto borderSprite = std::make_unique<cRectBorderMultiSprite>(1);
    borderSprite->SetZOrder(101);
    borderSprite->SetWindow(mTarget.mWindow);
    borderSprite->SetRGBColor(mConfig.mColors.mCodeBlockBorder);
    borderSprite->SetPosition(cPoint(0, 0));
    borderSprite->SetSize(cPoint(mTarget.mWidth, position.y));
    result.mSprites.emplace_back(std::move(borderSprite));

    result.mHeight = position.y + font.height();

    return result;
}

std::vector<cTextRenderer2Block> cTextRenderer2::parse(const std::string& text)
{
    std::vector<cBlock> blocks;
    bool isInsideCodeBlock = false;
    int numberOfCodeBlocks = 0, numberOfLinks = 0;
    const char* startOfCodeBlock = nullptr;
    eAlign defaultAlign = eAlign::Left;

    for (auto lineView : text | std::views::split('\n'))
    {
        std::string_view line(lineView.data(), lineView.size());
        if (line.starts_with("```")) // code block
        {
            if (isInsideCodeBlock) // end of code block:
            {                
                isInsideCodeBlock = false;
                auto& codeBlock = blocks.back();
                ASSERT(codeBlock.mIsCodeBlock);
                if(!startOfCodeBlock) 
                {
                    blocks.pop_back();
                    continue; // empty code block, ignore it
                }
                auto& codeBlockSpan = codeBlock.mSpans.emplace_back(); // code blocks has exactly one span
                codeBlockSpan.mCodeBlockIndex = numberOfCodeBlocks++;
                codeBlockSpan.mText = std::string_view(startOfCodeBlock, line.data() - startOfCodeBlock - 1);
                blocks.emplace_back();
            }
            else // start of code block:
            {
                if(blocks.empty() || !blocks.back().mSpans.empty())
                {
                    blocks.emplace_back();
                }
                auto& codeBlock = blocks.back();
                codeBlock.mIsCodeBlock = true;
                isInsideCodeBlock = true;
                startOfCodeBlock = nullptr;
                if (line.length() > 3)
                {
                    auto colonPos = line.find(':');
                    if (colonPos != std::string_view::npos)
                    {
                        codeBlock.mCodeBlock.mType = line.substr(3, colonPos - 3);
                        codeBlock.mCodeBlock.mTitle = line.substr(colonPos + 1);
                    }
                    else
                    {
                        codeBlock.mCodeBlock.mTitle = line.substr(3);
                    }
                }
            }
            continue;
        }
        if(isInsideCodeBlock)
        {
            if (startOfCodeBlock == nullptr)
            {
                startOfCodeBlock = line.data();
            }
            continue; // ignore line inside code block
        }
        if(blocks.empty() || line.empty())
        {
            blocks.emplace_back();
            blocks.back().mAlign = defaultAlign;
        }
        if (line.starts_with("-")) // list item
        {
            if(line.size()>=1 && line[1] == '-') // is comment?
            {
                continue;
            }
            line.remove_prefix(line.size() == 1 || line[1] != ' ' ? 1 : 2);
            if(!blocks.back().mSpans.empty())
            {
                blocks.back().mEmptyLineAfter = false;
                blocks.emplace_back();
                blocks.back().mAlign = defaultAlign;
            }
            blocks.back().mIsListItem = true;
        }
        auto& block = blocks.back();
        cSpan span;
        auto pushSpan = [&]()
            {
                if (!span.mText.empty())
                {
                    if (span.mIsLink) // find the link id. format: [[link_id:link_text]]
                    {
                        auto colonPos = span.mText.find(':');
                        if (colonPos != std::string_view::npos)
                        {
                            span.mLinkId = span.mText.substr(0, colonPos);
                            span.mText = span.mText.substr(colonPos + 1);
                        }
                        else // the link id is the text:
                        {
                            span.mLinkId = span.mText;
                        }
                    }
                    block.mSpans.push_back(span);
                }
                span.mText = {};
            };
        auto extendSpan([&](std::string_view lastWord)
            {
                if (span.mText.empty())
                {
                    span.mText = lastWord;
                }
                else
                {
                    span.mText = std::string_view(span.mText.data(), lastWord.data() + lastWord.size() - span.mText.data());
                }
            });
        while(!line.empty())
        {
            auto wordEnd = line.find_first_of(" \t");
            std::string_view word = wordEnd == std::string_view::npos ? line : line.substr(0, wordEnd);
            char separator = wordEnd == std::string_view::npos ? 0 : line[wordEnd];
            cPrefixRemover prefixRemover(line, std::min(word.size() + 1, line.size()));
            if (word.starts_with("@h") && word.size() >= 3)
            {
                pushSpan();
                block.mHeadingLevel = word[2] - '0';
                if (block.mHeadingLevel == 1)
                {
                    block.mAlign = cTextRenderer2Block::eAlign::Center; // the top heading is centered by default
                }
                continue; // ignore rest of the word
            }
            if(word.starts_with("@t") && word.size() >= 3)
            {
                pushSpan();
                block.mTabWidth = word[2] - '0';
                if (word.size() >= 4)
                {
                    block.mTabWidth = block.mTabWidth * 10 + word[3] - '0';
                }
                continue; // ignore rest of the word
            }
            auto setAlign = [&](eAlign align)
                {
                    pushSpan();
                    block.mAlign = align;
                    if (word.length() >= 3 && word[2] == '+')
                    {
                        defaultAlign = align;
                    }
                };
            if (word.starts_with("@|"))
            {
                setAlign(cTextRenderer2Block::eAlign::Center);
                continue; // ignore rest of the word
            }
            if (word.starts_with("@<"))
            {
                setAlign(cTextRenderer2Block::eAlign::Left);
                continue; // ignore rest of the word
            }
            if (word.starts_with("@>"))
            {
                setAlign(cTextRenderer2Block::eAlign::Right);
                continue; // ignore rest of the word
            }
            if (word.starts_with("@="))
            {
                setAlign(cTextRenderer2Block::eAlign::Justify);
                continue; // ignore rest of the word
            }
            if([&]() 
                {
                    for (int i = 1; i < word.size(); ++i)
                    {
                        if (word[i] == word[i - 1] &&
                            (word[i] == '`' || word[i] == '*' || word[i] == '_' || word[i] == '[' || word[i] == ']'))
                        {
                            extendSpan(word.substr(0, i - 1));
                            span.mSeparator = i == word.size() - 1 ? separator : 0;
                            pushSpan();
                            switch (word[i])
                            {
                            case '`':
                                span.mIsMonospace = !span.mIsMonospace;
                                break;
                            case '*':
                                span.mIsBold = !span.mIsBold;
                                break;
                            case '_':
                                span.mIsItalic = !span.mIsItalic;
                                break;
                            case '[': [[fallthrough]];
                            case ']':
                                span.mIsLink = !span.mIsLink;
                                break;
                            default:
                                break;
                            }
                            prefixRemover.overridePrefixSize(i + 1);
                            return true;
                        }
                    }
                    return false;
                }())
            {
                continue;
            }
            extendSpan(word);
        } // words
        pushSpan();
    }
    return blocks;
}

cTextRenderer2::cPrefixRemover::~cPrefixRemover()
{
    mText.remove_prefix(mPrefixSize);
}

void cTextRenderer2::cPrefixRemover::overridePrefixSize(int size)
{
    mPrefixSize = size;
}

} // namespace Pixie