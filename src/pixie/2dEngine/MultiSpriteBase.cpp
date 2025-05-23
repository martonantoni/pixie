#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
void cMultiSpriteBase::PropertiesChanged(unsigned int Properties)
{
	if (mSprites.empty())
	{
		return;
	}
	auto ForEachSprite=[this](auto Func) 
	{ 
		for(auto &Sprite: mSprites)
			if(Sprite) 
				Func(*Sprite);
	};

	if(Properties&Property_Rect)
	{
		ArrangeSprites();
	}
	if(Properties&Property_PositionOffset)
	{
		ForEachSprite([PositionOffset=mPositionOffset](cSpriteBase &Sprite) { Sprite.SetPositionOffset(PositionOffset); });
	}
	if(Properties&Property_Color)
	{
		ForEachSprite([NewColor=mProperties.mColor.GetRGBColor()](cSpriteBase &Sprite) { Sprite.SetRGBColor(NewColor); });
	}
	if(Properties&Property_Alpha)
	{
		ForEachSprite([Alpha=mProperties.mColor.GetAlpha()](cSpriteBase &Sprite) { Sprite.SetAlpha(Alpha); });
	}
	if(Properties&Property_ZOrder)
	{
		auto Change=mProperties.mZOrder-mSprites.front()->GetZOrder();
		ForEachSprite([Change](cSpriteBase &Sprite) { Sprite.SetZOrder(Sprite.GetZOrder()+Change); });
	}
	if(Properties&Property_Window)
	{
		ForEachSprite([Window=mWindow](cSpriteBase &Sprite) { Sprite.SetWindow(Window); });
	}
	if(Properties&Property_Visibility)
	{
		if(IsVisible())
		{
			ForEachSprite([](cSpriteBase &Sprite) { Sprite.Show(); });
		}
		else
		{
			ForEachSprite([](cSpriteBase &Sprite) { Sprite.Hide(); });
		}
	}
	if(Properties&Property_ValidRect)
	{
		ForEachSprite([ValidRect = mProperties.mValidRect](cSpriteBase& Sprite) { Sprite.SetValidRect(ValidRect); });
	}
	if (Properties & Property_ClippingMode)
	{
		ForEachSprite([ClippingMode = mProperties.mClippingMode](cSpriteBase& Sprite) { Sprite.setClippingMode(ClippingMode); });    
	}
}

cSimpleMultiSprite::cSimpleMultiSprite(std::vector<std::unique_ptr<cSpriteBase>> &&Sprites, eBasePosition basePosition)
{
	mSprites=std::move(Sprites);
    if (!mSprites.empty())
    {
        mRect = mSprites.front()->GetRect();
        for (int i = 1, iend = mSprites.size(); i != iend; ++i)
        {
            mRect.growToBound(mSprites[i]->GetRect());
        }
        mBasePos = basePosition == eBasePosition::AdjustedToBoundingBox ? mRect.position() : cPoint { 0, 0 };
    }
}

void cSimpleMultiSprite::ArrangeSprites()
{
	cPoint Change=GetPosition()-mBasePos;
	for(auto &Sprite: mSprites)
	{
		Sprite->SetPosition(Sprite->GetPosition()+Change);
	}
	mBasePos=GetPosition();
}

void cAutoMultiSpriteBase::Cleanup()
{
	mSprites.clear();
}

void cAutoMultiSpriteBase::OverrideTexture(size_t Index, tIntrusivePtr<cTexture> Texture)
{
	if(mTextureOverrides.size()<=Index)
		mTextureOverrides.resize(Index+1);
	mTextureOverrides[Index]=std::move(Texture);
}

bool cAutoMultiSpriteBase::Init(cPixieWindow *Window, const std::string &TextureNameBase)
{
	cTextureManager2 &TextureManager=theTextureManager;
	const char **PositionNames=GetPositionNames();
	size_t Index=0;
	for(const char **PositionName=PositionNames; *PositionName!=nullptr; ++PositionName, ++Index)
	{
		const char *ThisPositionName=*PositionName;
		bool IsOptional=*ThisPositionName=='*';
		if(IsOptional)
			++ThisPositionName;
		tIntrusivePtr<cTexture> Texture=mTextureOverrides.size()>Index&&mTextureOverrides[Index]?
			mTextureOverrides[Index]
			:
			(TextureManager.GetTexture(fmt::sprintf("%s_%s", TextureNameBase.c_str(), ThisPositionName), IsOptional));
		if(!Texture.get()&&IsOptional)
		{
			mSprites.push_back(nullptr);
			continue;
		}
		ASSERT(Texture.get());
		if(Texture.get())
		{
			auto Sprite=make_unique<cSprite>();
			Sprite->SetWindow(Window);
			Sprite->SetTextureAndSize(Texture);
			mSprites.push_back(std::move(Sprite));
		}
		else
		{
			Cleanup();
			return false;
		}
	}
	return FinalizeInit();
}

void cMultiSpriteBase::PositionSprite(cSpriteBase &Sprite, cPoint Position, ePositioning Positioning)
{
	switch(Positioning)
	{
	case ePositioning::TopLeft:
		Sprite.SetPosition(Position);
		break;
	case ePositioning::TopRight: 
		Sprite.SetPosition(Position-cPoint {Sprite.GetWidth(), 0});
		break;
	case ePositioning::BottomLeft: 
		Sprite.SetPosition(Position-cPoint { 0, Sprite.GetHeight() });
		break;
	case ePositioning::BottomRight:
		Sprite.SetPosition(Position-cPoint { Sprite.GetWidth(), Sprite.GetHeight() });
		break;
	}
}

void cMultiSpriteBase::StretchSpriteBetween(cSpriteBase &Sprite, const cRect &RectTopLeft, const cRect &RectBottomRight, eStretchInclusion StretchInclusion)
{
	ASSERT(RectTopLeft.size()==RectBottomRight.size());

 	bool HasVerticalDiff=(RectBottomRight.top()-RectTopLeft.top())!=0;
 	bool HasHorizontalDiff=(RectBottomRight.left()-RectTopLeft.left())!=0;
	switch(StretchInclusion)
	{
	case eStretchInclusion::ExcludeRects:
		if(!HasVerticalDiff)
		{
			PositionSprite(Sprite, RectTopLeft.topRight()+cPoint { 1,0 }, ePositioning::TopLeft);
			Sprite.SetSize(RectBottomRight.left()-RectTopLeft.right()-1, Sprite.GetHeight());
		}
		else
		{
			if(!HasHorizontalDiff)
			{
				PositionSprite(Sprite, RectTopLeft.bottomLeft()+cPoint { 0,1 }, ePositioning::TopLeft);
				Sprite.SetSize(Sprite.GetWidth(), RectBottomRight.top()-RectTopLeft.bottom()-1);
			}
			else
			{
				PositionSprite(Sprite, RectTopLeft.bottomLeft()+cPoint { 1,1 }, ePositioning::TopLeft);
				Sprite.SetSize(RectBottomRight.left()-RectTopLeft.right()-1, RectBottomRight.top()-RectTopLeft.bottom()-1);
			}
		}
		break;
	case eStretchInclusion::IncludeRects:
		if(!HasVerticalDiff)
		{
			PositionSprite(Sprite, RectTopLeft.topLeft(), ePositioning::TopLeft);
			Sprite.SetSize(RectBottomRight.right()-RectTopLeft.left(), Sprite.GetHeight());
		}
		else
		{
			if(!HasHorizontalDiff)
			{
				PositionSprite(Sprite, RectTopLeft.topLeft(), ePositioning::TopLeft);
				Sprite.SetSize(Sprite.GetWidth(), RectBottomRight.bottom()-RectTopLeft.top());
			}
			else
			{
				PositionSprite(Sprite, RectTopLeft.topLeft(), ePositioning::TopLeft);
				Sprite.SetSize(RectBottomRight.right()-RectTopLeft.left(), RectBottomRight.bottom()-RectTopLeft.top());
			}
		}
		break;
	}
}

void cMultiSpriteBase::CopyProperties(const cMultiSpriteBase& source)
{
    cSpriteBase::CopyProperties(source);
    for (auto& sprite : mSprites)
    {
        mSprites.emplace_back(sprite->Clone());
    }
}

std::unique_ptr<cSpriteBase> cMultiSpriteBase::Clone() const
{
    auto clone = std::make_unique<cMultiSpriteBase>();
    clone->CopyProperties(*this);
    return clone;
}


void cSimpleMultiSprite::CopyProperties(const cSimpleMultiSprite& source)
{
    mBasePos = source.mBasePos;
    cMultiSpriteBase::CopyProperties(source);
}

std::unique_ptr<cSpriteBase> cSimpleMultiSprite::Clone() const
{
    auto clone = std::make_unique<cSimpleMultiSprite>();
    clone->CopyProperties(*this);
    return clone;
}

cRectBorderMultiSprite::cRectBorderMultiSprite(int borderWidth):
	mBorderWidth(borderWidth)
{
	for (int i = 0; i < 4; ++i)
	{
		auto sprite = std::make_unique<cSprite>();
		sprite->SetTexture("1pix");
		mSprites.emplace_back(std::move(sprite));
	}
}

void cRectBorderMultiSprite::ArrangeSprites()
{
	cRect rect = GetRect();
	mSprites[spriteIndexes.top]->SetRect(cRect{ rect.left(), rect.top(), rect.width(), mBorderWidth });
	mSprites[spriteIndexes.bottom]->SetRect(cRect{ rect.left(), rect.bottom() - mBorderWidth, rect.width(), mBorderWidth });
	mSprites[spriteIndexes.left]->SetRect(cRect{ rect.left(), rect.top(), mBorderWidth, rect.height() });
	mSprites[spriteIndexes.right]->SetRect(cRect{ rect.right() - mBorderWidth, rect.top(), mBorderWidth, rect.height() });
}