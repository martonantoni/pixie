#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

void cSprite::SetTexture(tIntrusivePtr<cTexture> Texture)
{
	if (!CheckIfChangableProperty(Property_Texture))
		return;
	mTexture=std::move(Texture);
	PropertiesSet(Property_Texture);
}

void cSprite::SetTextureAndSize(tIntrusivePtr<cTexture> Texture)
{
	if(!Texture)
	{
		SetSize(0, 0);
	}
	else
	{
		SetSize({ Texture->GetTextureWidth(), Texture->GetTextureHeight() });
	}
	SetTexture(std::move(Texture));
}

void cSprite::SetBlendingMode(cSpriteRenderInfo::eBlendingMode BlendingFlags)
{
	mBlendingMode=BlendingFlags;
}

cSpriteRenderInfo cSprite::GetRenderInfo() const
{
    cSpriteRenderInfo renderInfo;
	renderInfo.mColor = mProperties.mColor;
	renderInfo.mRotation = GetRotation();
    renderInfo.mBlendingMode = mBlendingMode;
    renderInfo.mShader = mProperties.mShader.get();
	if (renderInfo.mShader)
	{
        for (int i = 0; i < 4; ++i)
            renderInfo.mShaderParameters[i] = mProperties.mShaderParameters[i];
	}
    renderInfo.mRect = GetRectForRendering();
    renderInfo.mTextures[0] = mTexture.get()->shaderResourceView();
    renderInfo.mTextureRects[0] = mTexture->GetTextureInfo();
    return renderInfo;
}

void cSprite::updateTextures()
{
    if (mTexture && mTexture->DoesNeedUpdateBeforeUse())
    {
        mTexture->Update();
    }
}

bool cSprite::SetStringProperty(unsigned int PropertyFlags, const std::string &Value)
{
	if(!CheckIfChangableProperty(PropertyFlags))
		return false;
	if(PropertyFlags==Property_Texture)
	{
		auto Texture=theTextureManager.GetTexture(Value);
		if(ASSERTTRUE(Texture))
		{
			SetTexture(Texture);
		}
		return true;
	}
	return cSpriteBase::SetStringProperty(PropertyFlags, Value);
}

bool cSprite::GetProperty(unsigned int PropertyFlags, OUT cPropertyValues &PropertyValues) const
{
	switch(PropertyFlags)
	{
	case Property_TextureSize: 
		if(mTexture)
		{
			PropertyValues=mTexture->GetSize();
			return true;
		}
		return false;
	default:
		return cSpriteBase::GetProperty(PropertyFlags, PropertyValues);
	}
}

void cSprite::CopyProperties(const cSprite& source)
{
    cSpriteBase::CopyProperties(source);
    mTexture = source.mTexture;
    mBlendingMode = source.mBlendingMode;
}

std::unique_ptr<cSpriteBase> cSprite::Clone() const
{
    auto clone = std::make_unique<cSprite>();
    clone->CopyProperties(*this);
    return clone;
}
