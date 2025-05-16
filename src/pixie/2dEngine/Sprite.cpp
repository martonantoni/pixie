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
	renderInfo.mCornerColors = mProperties.mColor.cornerColors();
	renderInfo.mRotation = GetRotation();
    renderInfo.mBlendingMode = mBlendingMode;
	if(mProperties.mClippingMode == eClippingMode::None)
	{
        renderInfo.mRect = GetRectForRendering();
        renderInfo.mTexture = mTexture.get();
        return renderInfo;
	}
// clipping is enabled:
	cRect OriginalRect = GetRect();
	OriginalRect.position() += GetPositionOffset();
	if (mWindow)
	{
		OriginalRect.position() += mWindow->GetScreenRect().position();
	}

	cRect validRect = mProperties.mValidRect;

	if (mWindow && mProperties.mClippingMode == eClippingMode::Parent)
	{
		validRect.position() += mWindow->GetScreenRect().position();
	}
	if (mWindow && mWindow->GetParentWindow() && mProperties.mClippingMode == eClippingMode::ParentParent)
	{
		validRect.position() += mWindow->GetParentWindow()->GetScreenRect().position();
	}
	if (validRect.isPointInside(OriginalRect.topLeft()) && validRect.isPointInside(OriginalRect.bottomRight()))
		return { GetRectForRendering(), GetRotation(), mTexture.get(), mBlendingMode };
	int Top = std::max(validRect.top(), OriginalRect.top());
	int Left = std::max(validRect.left(), OriginalRect.left());
	int Bottom = std::min(validRect.bottom(), OriginalRect.bottom());
	int Right = std::min(validRect.right(), OriginalRect.right());
	const cRect RenderedRect{ Left, Top, Right - Left + 1, Bottom - Top + 1 };
	if (RenderedRect.width() <= 0 || RenderedRect.height() <= 0)
	{
		return { RenderedRect, 0.0, nullptr, mBlendingMode };
	}
	const cRect TextureRect = mTexture->GetTextureRect();
	int ClippedTextureTop = TextureRect.top() + (RenderedRect.top() - OriginalRect.top()) * TextureRect.height() / OriginalRect.height();
	int ClippedTextureLeft = TextureRect.left() + (RenderedRect.left() - OriginalRect.left()) * TextureRect.width() / OriginalRect.width();
	int ClippedTextureBottom = TextureRect.bottom() - (OriginalRect.bottom() - RenderedRect.bottom()) * TextureRect.height() / OriginalRect.height();
	int ClippedTextureRight = TextureRect.right() - (OriginalRect.right() - RenderedRect.right()) * TextureRect.width() / OriginalRect.width();
	mClippedTexture = mTexture->CreateSubTexture(cRect{ ClippedTextureLeft, ClippedTextureTop, ClippedTextureRight - ClippedTextureLeft + 1, ClippedTextureBottom - ClippedTextureTop + 1 });

    renderInfo.mRect = RenderedRect;
    renderInfo.mTexture = mClippedTexture.get();
    return renderInfo;
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
