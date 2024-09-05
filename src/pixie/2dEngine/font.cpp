#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

cFont::cTextMetrics cFont::MeasureText(const UTF8::cDecodedString &Text) const
{
	int Width=0, WidthMax=0, LineCount=1;
	cTextMetrics Metrics;
	for(auto Letter: Text)
	{
		auto &LetterData=GetLetterData(Letter);
		Width+=LetterData.mAdvance>>6;
		if(Letter=='\n')
		{
			++LineCount;
			Metrics.mLineSizes.push_back({ Width,mFontHeight });
			if(WidthMax<Width)
				WidthMax=Width;
			Width=0;
		}
	}
	if(WidthMax<Width)
		WidthMax=Width;
	Metrics.mLineSizes.push_back({ Width,mFontHeight });
	Metrics.mBoundingSize=cPoint(WidthMax, mFontHeight*LineCount);
	return Metrics;
}

std::vector<std::unique_ptr<cSpriteBase>> cFont::CreateTextSprites(const UTF8::cDecodedString &Text)
{
	return CreateTextSprites(Text, { 0,0,0,0 }, eHorizontalAlign::Left, eVerticalAlign::Top);
}

std::vector<std::unique_ptr<cSpriteBase>> cFont::CreateTextSprites
	(const UTF8::cDecodedString &Text, const cRect &Rect, eHorizontalAlign HorizontalAlign, eVerticalAlign VerticalAlign) const
{
	int y=Rect.top();
	cTextMetrics TextMetrics=MeasureText(Text);
	std::vector<std::unique_ptr<cSpriteBase>> TextSprites;
	TextSprites.reserve(Text.size());
	switch(VerticalAlign)
	{
	case eVerticalAlign::Top:
		break;
	case eVerticalAlign::Bottom:
		y = Rect.bottom() - TextMetrics.mBoundingSize.y;
		break;
	case eVerticalAlign::Center:
		y += (Rect.height() - TextMetrics.mBoundingSize.y) / 2;
		break;
	}
	cPoint LetterPos(Rect.left(), y);
	size_t i=0, iend=Text.size();
	for(auto LineSize: TextMetrics.mLineSizes)
	{
		switch(HorizontalAlign)
		{
		case eHorizontalAlign::Left:
			LetterPos.x = Rect.left();
			break;
		case eHorizontalAlign::Right:
			LetterPos.x = Rect.right() - LineSize.x;
			break;
		case eHorizontalAlign::Center:
			LetterPos.x += (Rect.width() - LineSize.x) / 2;
			break;
		}
		for(; i<iend; ++i)
		{
			if(Text[i]=='\n')
			{
				++i;
				LetterPos.y+=mFontHeight;
				break;
			}
			const cLetterData &LetterData=GetLetterData(Text[i]);
			if(ASSERTFALSE(!LetterData.mTexture))
				continue;
			auto LetterSprite=std::make_unique<cSprite>();
			LetterSprite->SetPosition(LetterPos + cPoint(LetterData.mXOffset, LetterData.mYOffset));
			LetterSprite->SetTextureAndSize(LetterData.mTexture);
			TextSprites.emplace_back(std::move(LetterSprite));
			LetterPos.x+=LetterData.mAdvance>>6;
		}
	}
	return TextSprites;
}

tIntrusivePtr<cTexture> cFont::CreateTexture(const UTF8::cDecodedString &Text) const
{
	if(Text.empty())
		return nullptr;
	auto Sprites=CreateTextSprites(Text, cRect(0,0,0,0), eHorizontalAlign::Left, eVerticalAlign::Top);
	int RightMax=0, BottomMax=0;
	for(auto &Sprite: Sprites)
	{
		int SpriteRight=Sprite->GetRect().right();
		if(SpriteRight>RightMax)
			RightMax=SpriteRight;
		int SpriteBottom=Sprite->GetRect().bottom();
		if(SpriteBottom>BottomMax)
			BottomMax=SpriteBottom;
	}
	auto Texture=cSpriteRenderedTexture::Create({ RightMax+1, BottomMax+1 });
	for(auto &Sprite: Sprites)
	{
		((cSprite *)Sprite.get())->SetBlendingMode(cSpriteRenderInfo::Blend_CopySource); //todo fix ugly hack
		Texture->AddSprite(std::move(Sprite));
	}
	return Texture;
}

int cFont::GetAdvance(int Char) const
{
	if(ASSERTFALSE(Char<0||Char>=128))
		return 0;
	return mNormalLetters[Char].mAdvance>>6;
}

const cFont::cLetterData &cFont::GetLetterData(int Char) const
{
	if(ASSERTFALSE(Char<0||Char>=128))
		Char=0;
	return mNormalLetters[Char];
}

const cFont::cLetterData &cFont::GetLetterData(wchar_t Char) const
{
	if(Char>=0&&Char<128)
		return mNormalLetters[Char];
	auto i=mExtraLetters.find(Char);
	if(i==mExtraLetters.end())
		return mNormalLetters[0];
	return i->second;
}
