#include "StdAfx.h"

#include "VisualizationHelpers.h"

std::vector<std::unique_ptr<cSpriteBase>> CreateTileFillSprites(const tIntrusivePtr<cTexture> &Texture, cPoint Size)
{
	std::vector<std::unique_ptr<cSpriteBase>> Sprites;
	int tw=Texture->GetTextureWidth(),th=Texture->GetTextureHeight();
	Sprites.reserve((Size.y/th+1)*(Size.x/tw+1));
	for(int x=0;x<Size.x;x+=tw)
	{
		for(int y=0;y<Size.y;y+=th)
		{
			auto Sprite=std::make_unique<cSprite>();
			int Width=std::min(tw, Size.x-x);
			int Height=std::min(th, Size.y-y);
			Sprite->SetPosition({ x, y });
			Sprite->SetSize(Width, Height);
			Sprite->SetTexture(Width==tw&&Height==th?Texture:Texture->CreateSubTexture(cRect(0,0,Width,Height)));
			Sprites.emplace_back(std::move(Sprite));
		}
	}
	return Sprites;
}

std::vector<std::unique_ptr<cSpriteBase>> CreateTileFillSprites(const tIntrusivePtr<cTexture> &Texture, cPoint Size, cPoint TextureOffset)
{ // for perfomance reasons it is kept as a separate function from the non-offseted version
	std::vector<std::unique_ptr<cSpriteBase>> Sprites;
	int tw=Texture->GetTextureWidth(), th=Texture->GetTextureHeight();
	TextureOffset.x%=tw;
	TextureOffset.y%=th;
	Sprites.reserve((Size.y/th+2)*(Size.x/tw+2));

	int TextureWidthToUse=tw-TextureOffset.x;
	int TextureXOffset=TextureOffset.x;

	for(int x=0; x<Size.x; )
	{
		int TextureHeightToUse=th-TextureOffset.y;
		int TextureYOffset=TextureOffset.y;
		for(int y=0; y<Size.y; )
		{
			auto Sprite=std::make_unique<cSprite>();
			int Width=std::min(TextureWidthToUse, Size.x-x);
			int Height=std::min(TextureHeightToUse, Size.y-y);
			Sprite->SetPosition({ x, y });
			Sprite->SetSize(Width, Height);
			Sprite->SetTexture(Width==tw&&Height==th?Texture:Texture->CreateSubTexture(cRect(TextureXOffset, TextureYOffset, Width, Height)));
			y+=TextureHeightToUse;
			TextureHeightToUse=th;
			TextureYOffset=0;
			Sprites.emplace_back(std::move(Sprite));
		}
		x+=TextureWidthToUse;
		TextureWidthToUse=tw;
		TextureXOffset=0;
	}
	return Sprites;
}
