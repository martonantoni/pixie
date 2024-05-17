#pragma once

void SpriteBetweenPoints(cSprite &Sprite, cPoint a, cPoint b, int width = 1);
std::unique_ptr<cSprite> CreateLineSprite(cPoint a, cPoint b, int width = 1);
