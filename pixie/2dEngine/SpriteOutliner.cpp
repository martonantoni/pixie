#include "StdAfx.h"

cSpriteOutliner::cSpriteOutliner(cSpriteBase& outlinedSprite)
    : mOutlinedSprite(outlinedSprite)
{
    auto outlineSize = outlinedSprite.GetSize() + cPoint{ 2, 2 };
    auto texture = cSpriteRenderedTexture::Create(outlineSize);
    auto cloneOfOutlinedSprite = outlinedSprite.Clone();
    cloneOfOutlinedSprite->SetPosition({ 1,1 });
    texture->AddSprite(std::move(cloneOfOutlinedSprite));
}