#pragma once

class cSpriteOutliner
{
    cSpriteBase& mOutlinedSprite;
    std::unique_ptr<cSprite> mOutline;
public:
    cSpriteOutliner(cSpriteBase& outlinedSprite);
};