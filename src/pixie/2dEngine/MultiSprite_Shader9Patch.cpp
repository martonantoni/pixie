#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

#include "pixie/pixie/2dEngine/MultiSprite_Shader9Patch.h"

void cMultiSprite_Shader9Patch::ArrangeSprites()
{
    if (mSprites.size() < 8)
    {
        return;
    }
    if(!mFrameWidths.isInitialized())
    {
        mFrameWidths.mTop = mSprites[Indexes::top]->getShader()->config().get<int>("width");
        mFrameWidths.mBottom = mSprites[Indexes::bottom]->getShader()->config().get<int>("width");
        mFrameWidths.mLeft = mSprites[Indexes::left]->getShader()->config().get<int>("width");
        mFrameWidths.mRight = mSprites[Indexes::right]->getShader()->config().get<int>("width");
    }
    cRect usedRect = GetRect();
    if (mRectSpecifiesCenter)
    {
        usedRect.left() -= mFrameWidths.mLeft;
        usedRect.right() += mFrameWidths.mRight;
        usedRect.top() -= mFrameWidths.mTop;
        usedRect.bottom() += mFrameWidths.mBottom;
    }
    mSprites[Indexes::topLeft]->SetPosition(0, 0);
    mSprites[Indexes::topLeft]->SetSize(mFrameWidths.mLeft, mFrameWidths.mTop);
    mSprites[Indexes::top]->SetPosition(mFrameWidths.mLeft, 0);
    mSprites[Indexes::top]->SetSize(usedRect.width() - mFrameWidths.mLeft - mFrameWidths.mRight, mFrameWidths.mTop);
    mSprites[Indexes::topRight]->SetPosition(usedRect.width() - mFrameWidths.mRight, 0);
    mSprites[Indexes::topRight]->SetSize(mFrameWidths.mRight, mFrameWidths.mTop);
    mSprites[Indexes::left]->SetPosition(0, mFrameWidths.mTop);
    mSprites[Indexes::left]->SetSize(mFrameWidths.mLeft, usedRect.height() - mFrameWidths.mTop - mFrameWidths.mBottom);
    mSprites[Indexes::right]->SetPosition(usedRect.width() - mFrameWidths.mRight, mFrameWidths.mTop);
    mSprites[Indexes::right]->SetSize(mFrameWidths.mRight, usedRect.height() - mFrameWidths.mTop - mFrameWidths.mBottom);
    mSprites[Indexes::bottomLeft]->SetPosition(0, usedRect.height() - mFrameWidths.mBottom);
    mSprites[Indexes::bottomLeft]->SetSize(mFrameWidths.mLeft, mFrameWidths.mBottom);
    mSprites[Indexes::bottom]->SetPosition(mFrameWidths.mLeft, usedRect.height() - mFrameWidths.mBottom);
    mSprites[Indexes::bottom]->SetSize(usedRect.width() - mFrameWidths.mLeft - mFrameWidths.mRight, mFrameWidths.mBottom);
    mSprites[Indexes::bottomRight]->SetPosition(usedRect.width() - mFrameWidths.mRight, usedRect.height() - mFrameWidths.mBottom);
    mSprites[Indexes::bottomRight]->SetSize(mFrameWidths.mRight, mFrameWidths.mBottom);
    if (mSprites.size() == 9)
    {
        mSprites[Indexes::center]->SetPosition(mFrameWidths.mLeft, mFrameWidths.mTop);
        mSprites[Indexes::center]->SetSize(usedRect.width() - mFrameWidths.mLeft - mFrameWidths.mRight, usedRect.height() - mFrameWidths.mTop - mFrameWidths.mBottom);
    }
}

std::unique_ptr<cMultiSprite_Shader9Patch> cMultiSprite_Shader9Patch::create(const cInitData& initData)
{
    cMultiSprite::cSprites sprites;
    auto addSprite = [&](std::string_view positionName)
        {
            auto sprite = std::make_unique<cSprite>();
            sprite->setShader(std::format("{}.{}", initData.mShaderFolderName, positionName));
            sprite->SetWindow(initData.mWindow);
            sprites.emplace_back(std::move(sprite));
        };
   
    addSprite("top_left");
    addSprite("top");
    addSprite("top_right");
    addSprite("left");
    addSprite("right");
    addSprite("bottom_left");
    addSprite("bottom");
    addSprite("bottom_right");

    return std::make_unique<cMultiSprite_Shader9Patch>(std::move(sprites), initData.mRectSpecifiesCenter);
}
