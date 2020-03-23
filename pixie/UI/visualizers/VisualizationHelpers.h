#pragma once

#include "VisualizationHelpers.h"

std::vector<std::unique_ptr<cSpriteBase>> CreateTileFillSprites(const tIntrusivePtr<cTexture> &Texture, cPoint Size);
std::vector<std::unique_ptr<cSpriteBase>> CreateTileFillSprites(const tIntrusivePtr<cTexture> &Texture, cPoint Size, cPoint TextureOffset);

