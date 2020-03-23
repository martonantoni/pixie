#include "StdAfx.h"

cFont2::cTextMetrics cFont2::MeasureText(const UTF8::cDecodedString &Text) const
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

std::vector<std::unique_ptr<cSpriteBase>> cFont2::CreateTextSprites(const UTF8::cDecodedString &Text)
{
	return CreateTextSprites(Text, { 0,0,0,0 }, eHorizontalAlign::Left, eVerticalAlign::Top);
}

std::vector<std::unique_ptr<cSpriteBase>> cFont2::CreateTextSprites
	(const UTF8::cDecodedString &Text, const cRect &Rect, eHorizontalAlign HorizontalAlign, eVerticalAlign VerticalAlign) const
{
	int y=Rect.mTop;
	cTextMetrics TextMetrics=MeasureText(Text);
	std::vector<std::unique_ptr<cSpriteBase>> TextSprites;
	TextSprites.reserve(Text.size());
	switch(VerticalAlign)
	{
	case eVerticalAlign::Top:
		break;
	case eVerticalAlign::Bottom:
		y=Rect.Bottom()-TextMetrics.mBoundingSize.y;
		break;
	case eVerticalAlign::Center:
		y+=(Rect.mHeight-TextMetrics.mBoundingSize.y)/2;
		break;
	}
	cPoint LetterPos(Rect.mLeft, y);
	size_t i=0, iend=Text.size();
	for(auto LineSize: TextMetrics.mLineSizes)
	{
		switch(HorizontalAlign)
		{
		case eHorizontalAlign::Left:
			LetterPos.x=Rect.mLeft;
			break;
		case eHorizontalAlign::Right:
			LetterPos.x=Rect.Right()-LineSize.x;
			break;
		case eHorizontalAlign::Center:
			LetterPos.x+=(Rect.mWidth-LineSize.x)/2;
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

tIntrusivePtr<cTexture> cFont2::CreateTexture(const UTF8::cDecodedString &Text) const
{
	if(Text.empty())
		return nullptr;
	auto Sprites=CreateTextSprites(Text, cRect(0,0,0,0), eHorizontalAlign::Left, eVerticalAlign::Top);
	int RightMax=0, BottomMax=0;
	for(auto &Sprite: Sprites)
	{
		int SpriteRight=Sprite->GetRect().Right();
		if(SpriteRight>RightMax)
			RightMax=SpriteRight;
		int SpriteBottom=Sprite->GetRect().Bottom();
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

int cFont2::GetAdvance(int Char) const
{
	if(ASSERTFALSE(Char<0||Char>=128))
		return 0;
	return mNormalLetters[Char].mAdvance>>6;
}

const cFont2::cLetterData &cFont2::GetLetterData(int Char) const
{
	if(ASSERTFALSE(Char<0||Char>=128))
		Char=0;
	return mNormalLetters[Char];
}

const cFont2::cLetterData &cFont2::GetLetterData(wchar_t Char) const
{
	if(Char>=0&&Char<128)
		return mNormalLetters[Char];
	auto i=mExtraLetters.find(Char);
	if(i==mExtraLetters.end())
		return mNormalLetters[0];
	return i->second;
}
