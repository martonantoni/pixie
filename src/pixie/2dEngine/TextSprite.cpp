#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
void cTextSprite::ConstructorCommon()
{
	mLockedProperties|=Property_PositionOffset;
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
	if(mTextures[0])
	{
		cRect AlignedRect = cRect::alignedRect(
			cRect{ {0,0}, GetSize() }, 
			cRect{ {0,0}, mTextures[0]->GetSize() },
			mHorizontalAlign, mVeritcalAlign);
		mPositionOffset=AlignedRect.position();
	}
}

cSpriteRenderInfo cTextSprite::GetRenderInfo() const
{
	auto RenderInfo=cSprite::GetRenderInfo();
	if(mTextures[0] && !mIsResizable)
		RenderInfo.mRect.size() = mTextures[0]->GetSize();
	return RenderInfo;
}

void cTextSprite::SetResizable(bool Resizable)
{
    if (Resizable && !mIsResizable)
    {
        if (mTextures[0])
        {
            SetSize(mTextures[0]->GetSize());
        }
    }
    mIsResizable = Resizable;
}

void cTextSprite::SetSizeToTextureSize()
{
    UpdateTexture();
    SetSize(mTextures[0]->GetSize());
}

void cTextSprite::SetFont(std::shared_ptr<const cFont> Font)
{
	mFont=std::move(Font);
	if(mVisible&&mWindow)
		NeedTextureUpdate();
	else
		setTexture(tIntrusivePtr<cTexture>());
}

void cTextSprite::SetFont(const std::string &FontName)
{
	SetFont(theFontManager.font(FontName));
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
	if(mVisible&&mWindow)
		NeedTextureUpdate();
	else
		setTexture(tIntrusivePtr<cTexture>());
}

void cTextSprite::UpdateTexture()
{
	if(mFont)
	{
		setTexture(mFont->CreateTexture(mText));
		HandleAlignment();
	}
}

void cTextSprite::PropertiesChanged(unsigned int Properties)
{
	if(Properties&Property_Visibility&&IsVisible()&&!mTextures[0])
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
