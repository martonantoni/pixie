#include "StdAfx.h"

#include "pixie/pixie/2dEngine/text_renderer_2.h"

namespace Pixie
{

void cTextRenderer2::init(const cTextRenderer2Config& config)
{
    mConfig = config;
}

std::pair<const cFont&, const cColor&>  cTextRenderer2::determineFont(const cTextRenderer2Block& block, const cTextRenderer2Span& span) const
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

int cTextRenderer2::getNextTabStop(int x) const
{
    for (int tabStop : mConfig.mTabStops)
    {
        if (tabStop > x)
        {
            return tabStop;
        }
    }
    int tabWidth = mConfig.mTabWidth * mConfig.mFonts.mRegular->letterData(' ').advance();
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
        int centeringOffset = (mConfig.mWidth - wordsWidth) / 2;
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
        int rightOffset = mConfig.mWidth - wordsWidth;
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
        int missingSpace = mConfig.mWidth - wordsWidth;
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

    for (auto& span : block.mSpans)
    {
        auto [font, color] = determineFont(block, span);
        int spaceWidth = font.letterData(' ').advance();
        int firstSpriteInSpan = sprites.size();
        auto text = span.mText;
        bool wasTabBefore = false;
        while(!text.empty())
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
            cPoint position(0, 0);
            while (!textWord.empty())
            {
                wchar_t decodedChar = UTF8::popCharacter(textWord);
                auto& letterData = font.letterData(decodedChar);
                auto sprite = std::make_unique<cSprite>();
                sprite->SetTextureAndSize(letterData.mTexture);
                sprite->SetWindow(mConfig.mWindow);
                sprite->SetRGBColor(color);
                sprite->SetPosition(position + letterData.offset());
                sprites.emplace_back(std::move(sprite));
                position.x += letterData.advance();
            }
            word.mLastSpriteIndex = sprites.size() - 1;
            word.mWidth = position.x;
        }
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
    for (auto [idx, word] : std::ranges::views::enumerate(mWords))
    {
        if(x + word.width() > mConfig.mWidth)
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
            x = getNextTabStop(x);
        }
        else
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
    result.mHeight = lineYOffset + mConfig.mFonts.mRegular->height();

    return result;
}

cTextRenderer2BlockResult cTextRenderer2::renderCodeBlock(const cBlock& block)
{
    cTextRenderer2BlockResult result;
    ASSERT(block.mSpans.size() == 1);
    auto& span = block.mSpans[0];
    const cFont& font = mConfig.mFonts.mMonospace ? *mConfig.mFonts.mMonospace : *mConfig.mFonts.mRegular;
    int startX = font.letterData(' ').advance();
    cPoint position(startX, font.height() / 2);
    for (auto lineView : span.mText | std::ranges::views::split('\n'))
    {
        std::string_view line(lineView.data(), lineView.size());
        while (!line.empty())
        {
            wchar_t decodedChar = UTF8::popCharacter(line);
            auto& letterData = font.letterData(decodedChar);
            auto sprite = std::make_unique<cSprite>();
            sprite->SetTextureAndSize(letterData.mTexture);
            sprite->SetZOrder(100);
            sprite->SetWindow(mConfig.mWindow);
            sprite->SetRGBColor(mConfig.mColors.mDefaultColor);
            sprite->SetPosition(position + letterData.offset());
            result.mSprites.emplace_back(std::move(sprite));
            position.x += letterData.advance();
        }
        position.y += font.height();
        position.x = startX;
    }
    position.y += font.height() / 2;
    // add background sprite:
    auto backgroundSprite = std::make_unique<cSprite>();
    backgroundSprite->SetTexture("1pix");
    backgroundSprite->SetZOrder(99);
    backgroundSprite->SetWindow(mConfig.mWindow);
    backgroundSprite->SetRGBColor(mConfig.mColors.mCodeBlockBG);
    backgroundSprite->SetPosition(cPoint(0, 0));
    backgroundSprite->SetSize(cPoint(mConfig.mWidth, position.y));
    result.mSprites.emplace_back(std::move(backgroundSprite));
    // add border sprite:
    auto borderSprite = std::make_unique<cRectBorderMultiSprite>(1);
    borderSprite->SetZOrder(101);
    borderSprite->SetWindow(mConfig.mWindow);
    borderSprite->SetRGBColor(mConfig.mColors.mCodeBlockBorder);
    borderSprite->SetPosition(cPoint(0, 0));
    borderSprite->SetSize(cPoint(mConfig.mWidth, position.y));
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
    for (auto lineView : text | std::ranges::views::split('\n'))
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
            }
            else // start of code block:
            {
                auto& codeBlock = blocks.emplace_back();
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
        auto& block = blocks.emplace_back();
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
            line.remove_prefix(std::min(word.size() + 1, line.size()));
            if (word.starts_with("@h") && word.size() >= 3)
            {
                pushSpan();
                block.mHeadingLevel = word[2] - '0';
                continue; // ignore rest of the word
            }
            if (word.starts_with("@|"))
            {
                pushSpan();
                block.mAlign = cTextRenderer2Block::eAlign::Center;
                continue; // ignore rest of the word
            }
            if (word.starts_with("@<"))
            {
                pushSpan();
                block.mAlign = cTextRenderer2Block::eAlign::Left;
                continue; // ignore rest of the word
            }
            if (word.starts_with("@>"))
            {
                pushSpan();
                block.mAlign = cTextRenderer2Block::eAlign::Right;
                continue; // ignore rest of the word
            }
            if (word.starts_with("@="))
            {
                pushSpan();
                block.mAlign = cTextRenderer2Block::eAlign::Justify;
                continue; // ignore rest of the word
            }
            if (word.starts_with("``"))
            {
                pushSpan();
                span.mIsMonospace = true;
                word.remove_prefix(2);
            }
            if (word.starts_with("[["))
            {
                pushSpan();
                span.mIsLink = true;
                word.remove_prefix(2);
            }
            if (word.starts_with("__"))
            {
                pushSpan();
                span.mIsItalic = true;
                word.remove_prefix(2);
            }
            if (word.starts_with("**"))
            {
                pushSpan();
                span.mIsBold = true;
                word.remove_prefix(2);
            }
            bool needPush = false, removeBold = false, removeItalic = false, removeLink = false, removeMonospace = false;
            if (word.ends_with("``"))
            {
                word.remove_suffix(2);
                extendSpan(word);
                needPush = true;
                removeMonospace = true;
            }
            if (word.ends_with("]]"))
            {
                word.remove_suffix(2);
                extendSpan(word);
                needPush = true;
                removeLink = true;
            }
            if (word.ends_with("__"))
            {
                word.remove_suffix(2);
                extendSpan(word);
                needPush = true;
                removeItalic = true;
            }
            if (word.ends_with("**"))
            {
                word.remove_suffix(2);
                extendSpan(word);
                needPush = true;
                removeBold = true;
            }

            extendSpan(word);
            if (needPush)
            {
                span.mSeparator = separator;
                pushSpan();
                if (removeBold)
                {
                    span.mIsBold = false;
                }
                if (removeItalic)
                {
                    span.mIsItalic = false;
                }
                if (removeLink)
                {
                    span.mIsLink = false;
                }
                if (removeMonospace)
                {
                    span.mIsMonospace = false;
                }
            }
        } // words
        pushSpan();
    }
    return blocks;
}

} // namespace Pixie