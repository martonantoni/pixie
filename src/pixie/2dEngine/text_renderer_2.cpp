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
    if (block.mIsCodeBlock || span.mIsMonospace)
    {
        return { *mConfig.mFonts.mMonospace, mConfig.mColors.mDefaultColor };
    }
    if(span.mIsBold)
    {
        if(span.mIsItalic)
        {
            return { *mConfig.mFonts.mBoldItalic, mConfig.mColors.mDefaultColor };
        }
        return { *mConfig.mFonts.mBold, mConfig.mColors.mDefaultColor };
    }
    if(span.mIsItalic)
    {
        return { *mConfig.mFonts.mItalic, mConfig.mColors.mDefaultColor };
    }
    if(block.mHeadingLevel > 0)
    {
        if (block.mHeadingLevel < mConfig.mFonts.mHeadings.size())
        {
            return { *mConfig.mFonts.mHeadings[block.mHeadingLevel - 1], mConfig.mColors.mDefaultColor };
        }
        return { *mConfig.mFonts.mBold, mConfig.mColors.mDefaultColor };
    }
    return { *mConfig.mFonts.mRegular, mConfig.mColors.mDefaultColor };
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
            for (int j = mWords[i].mFirstSpriteIndex; j <= mWords[i].mLastSpriteIndex; ++j)
            {
                auto pos = sprites[j]->GetPosition();
                pos.y += lineYOffset + ascender - mWords[i].mAscender;
                pos.x += lineXOffset;
                sprites[j]->SetPosition(pos);
            }
        }
        break;
    case cTextRenderer2Block::eAlign::Center:
    {
        int centeringOffset = (mConfig.mWidth - wordsWidth) / 2;
        for (int i = lineFirstWordIndex; i <= lineLastWordIndex; ++i)
        {
            for (int j = mWords[i].mFirstSpriteIndex; j <= mWords[i].mLastSpriteIndex; ++j)
            {
                auto pos = sprites[j]->GetPosition();
                pos.y += lineYOffset + ascender - mWords[i].mAscender;
                pos.x += lineXOffset + centeringOffset;
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
            for (int j = mWords[i].mFirstSpriteIndex; j <= mWords[i].mLastSpriteIndex; ++j)
            {
                auto pos = sprites[j]->GetPosition();
                pos.y += lineYOffset + ascender - mWords[i].mAscender;
                pos.x += lineXOffset + rightOffset;
                sprites[j]->SetPosition(pos);
            }
        }
        break;
    }
    case cTextRenderer2Block::eAlign::Justify:
    {
        int spaceCount = lineLastWordIndex - lineFirstWordIndex;
        int spaceWidth = (mConfig.mWidth - wordsWidth) / spaceCount;
        for (int i = lineFirstWordIndex; i <= lineLastWordIndex; ++i)
        {
            for (int j = mWords[i].mFirstSpriteIndex; j <= mWords[i].mLastSpriteIndex; ++j)
            {
                auto pos = sprites[j]->GetPosition();
                pos.y += lineYOffset + ascender - mWords[i].mAscender;
                pos.x += lineXOffset;
                sprites[j]->SetPosition(pos);
            }
            lineXOffset += spaceWidth;
        }
        break;
    }
    } // switch (block.mAlignment)
    return lineHeight;
}


cTextRenderer2BlockResult cTextRenderer2::render(const cTextRenderer2Block& block)
{
    mWords.clear();
    cTextRenderer2BlockResult result;
    auto& sprites = result.mSprites;

    cPoint position(0, 0);
    for (auto& span : block.mSpans)
    {
        auto [font, color] = determineFont(block, span);
        for(auto wordView: span.mText | std::ranges::views::split(' '))
        {
            std::string_view textWord(wordView.data(), wordView.size());
            cWord& word = mWords.emplace_back();
            word.mFirstSpriteIndex = sprites.size();
            word.mStartX = position.x;
            word.mHeight = font.height();
            word.mAscender = font.ascender();
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
            position.x += font.letterData(' ').advance();
            word.mEndX = position.x;
        }
    }
    int x = 0, lineXOffset = 0, lineYOffset = 0;
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
        x += word.width();
    }
    if (lineFirstWordIndex < mWords.size())
    {
        lineYOffset += arrangeWords(
            block.mAlign == eAlign::Justify ? eAlign::Left : block.mAlign,
            sprites, lineFirstWordIndex, mWords.size() - 1, lineYOffset);
    }
    result.mHeight = lineYOffset;

    return result;
}

std::vector<cTextRenderer2Block> cTextRenderer2::parse(const std::string& text)
{
    std::vector<cBlock> blocks;
    for (auto lineView : text | std::ranges::views::split('\n'))
    {
        std::string_view line(lineView.data(), lineView.size());
        if (line.starts_with("```")) // code block
        {
            // handle code block...
            continue;
        }
        auto& block = blocks.emplace_back();
        cSpan span;
        auto pushSpan = [&]()
            {
                if (!span.mText.empty())
                {
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
        for (auto wordView : line | std::ranges::views::split(' '))
        {
            std::string_view word(wordView.data(), wordView.size());
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
            bool needPush = false, removeBold = false, removeItalic = false, removeLink = false;
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
            }
        } // words
        pushSpan();
    }
    return blocks;
}

} // namespace Pixie