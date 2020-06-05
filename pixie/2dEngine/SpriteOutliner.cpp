#include "StdAfx.h"

cSpriteOutliner::cSpriteOutliner(cSpriteBase& outlinedSprite)
    : mOutlinedSprite(outlinedSprite)
{
    auto outlineSize = outlinedSprite.GetSize() + cPoint{ 2, 2 };
//    auto texture = cSpriteRenderedTexture::Create(outlineSize);
    auto targetTexture = cTexture::CreateWriteable(outlineSize);
    cTexture::cLockInfo targetLockInfo = targetTexture->LockSurface(cTexture::IsReadOnly::yes);
    for(int y=0;y<outlineSize.y;++y)
        memset(targetLockInfo.mBytes + y * targetLockInfo.mPitch, 0xff, targetTexture->GetSurfaceWidth() * 4);

//    for

    targetTexture->UnlockSurface();

    mOutline = std::make_unique<cSprite>();
    mOutline->SetWindow(outlinedSprite.GetWindow());
    mOutline->SetZOrder(outlinedSprite.GetZOrder() - 1);
    mOutline->SetPosition(outlinedSprite.GetPosition() - cPoint { 1, 1 });
    mOutline->SetTextureAndSize(std::move(targetTexture));
    mOutline->Show();

}