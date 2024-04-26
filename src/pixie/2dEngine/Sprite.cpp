#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
void cSprite::SetTexture(tIntrusivePtr<cTexture> Texture)
{
	mTexture=std::move(Texture);
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
	if(!mUseClipping)
	{
		return { GetRectForRendering(), GetRotation(), mTexture.get(), mBlendingMode };
	}
// clipping is enabled:
	cRect OriginalRect = GetRect();
	OriginalRect.Move(GetPositionOffset());
	if (mProperties.mValidRect.IsPointInside(OriginalRect.TopLeft()) && mProperties.mValidRect.IsPointInside(OriginalRect.BottomRight()))
		return { GetRectForRendering(), GetRotation(), mTexture.get(), mBlendingMode };
	int Top = std::max(mProperties.mValidRect.Top(), OriginalRect.Top());
	int Left = std::max(mProperties.mValidRect.Left(), OriginalRect.Left());
	int Bottom = std::min(mProperties.mValidRect.Bottom(), OriginalRect.Bottom());
	int Right = std::min(mProperties.mValidRect.Right(), OriginalRect.Right());
	cRect RenderedRect{ Left, Top, Right - Left + 1, Bottom - Top + 1 };
	if (RenderedRect.mWidth <= 0 || RenderedRect.mHeight <= 0)
	{
		return { RenderedRect, 0.0, nullptr, mBlendingMode };
	}
	cRect TextureRect = mTexture->GetTextureRect();
	int ClippedTextureTop = TextureRect.Top() + (RenderedRect.Top() - OriginalRect.Top()) * TextureRect.mHeight / OriginalRect.mHeight;
	int ClippedTextureLeft = TextureRect.Left() + (RenderedRect.Left() - OriginalRect.Left()) * TextureRect.mWidth / OriginalRect.mWidth;
	int ClippedTextureBottom = TextureRect.Bottom() - (OriginalRect.Bottom() - RenderedRect.Bottom()) * TextureRect.mHeight / OriginalRect.mHeight;
	int ClippedTextureRight = TextureRect.Right() - (OriginalRect.Right() - RenderedRect.Right()) * TextureRect.mWidth / OriginalRect.mWidth;
	mClippedTexture = mTexture->CreateSubTexture(cRect{ ClippedTextureLeft, ClippedTextureTop, ClippedTextureRight - ClippedTextureLeft + 1, ClippedTextureBottom - ClippedTextureTop + 1 });
	if (mWindow)
	{
		RenderedRect.Move(mWindow->GetScreenRect().GetPosition());
	}
	return { RenderedRect, GetRotation(), mClippedTexture.get(), mBlendingMode };
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
