#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
void cTextSprite::ConstructorCommon()
{
	mLockedProperties|=Property_PositionOffset;
// 	mProperties.mColor.SetRGBColor(0);
// 	mIsColorSet=false;
}

cTextSprite::cTextSprite()
{
	ConstructorCommon();
}

void cTextSprite::NeedTextureUpdate()
{
	if(!mUpdateTextureID.IsValid())
	{
		mUpdateTextureID=theRenderers.Register([this]() 
		{ 
			mUpdateTextureID.Unregister(); 
			UpdateTexture(); 
		});
	}
}

void cTextSprite::HandleAlignment()
{
	if(mTexture)
	{
		cRect AlignedRect=cRect({ 0,0 }, GetSize()).GetAlignedRect({ {0,0}, mTexture->GetSize() }, mHorizontalAlign, mVeritcalAlign);
		mProperties.mPositionOffset=AlignedRect.GetPosition();
	}
}

cSpriteRenderInfo cTextSprite::GetRenderInfo() const
{
	auto RenderInfo=cSprite::GetRenderInfo();
	if(mTexture && !mIsResizable)
		RenderInfo.mRect.SetSize(mTexture->GetSize());
	return RenderInfo;
}

void cTextSprite::SetResizable(bool Resizable)
{
    if (Resizable && !mIsResizable)
    {
        if (mTexture)
        {
            SetSize(mTexture->GetSize());
        }
    }
    mIsResizable = Resizable;
}

void cTextSprite::SetSizeToTextureSize()
{
    UpdateTexture();
    SetSize(mTexture->GetSize());
}

void cTextSprite::SetFont(std::shared_ptr<const cFont> Font)
{
	mFont=std::move(Font);
	if(mProperties.mVisible&&mWindow)
		NeedTextureUpdate();
	else
		SetTexture(tIntrusivePtr<cTexture>());
}

void cTextSprite::SetFont(const std::string &FontName)
{
	SetFont(theFontManager.GetFont(FontName));
}

void cTextSprite::SetAlignment(eHorizontalAlign Horizontal, eVerticalAlign Vertical)
{
	mHorizontalAlign=Horizontal;
	mVeritcalAlign=Vertical;
	HandleAlignment();
}

void cTextSprite::SetText(const std::string &Text)
{
	mText=Text;
	if(mProperties.mVisible&&mWindow)
		NeedTextureUpdate();
	else
		SetTexture(tIntrusivePtr<cTexture>());
}

void cTextSprite::UpdateTexture()
{
	if(mFont)
	{
		SetTexture(mFont->CreateTexture(mText));
		HandleAlignment();
	}
}

void cTextSprite::PropertiesChanged(unsigned int Properties)
{
	if(Properties&Property_Visibility&&IsVisible()&&!mTexture)
		NeedTextureUpdate();
}

bool cTextSprite::SetStringProperty(unsigned int PropertyFlags, const std::string &Value)
{
	if(!CheckIfChangableProperty(PropertyFlags))
		return false;
	if(PropertyFlags==Property_Text)
	{
		SetText(Value);
		return true;
	}
	return cSpriteBase::SetStringProperty(PropertyFlags, Value);
}
