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
	renderInfo.mColor = mColor;
	renderInfo.mRotation = GetRotation();
    renderInfo.mBlendingMode = mBlendingMode;
    renderInfo.mShader = mShader.get();
	if (renderInfo.mShader)
	{
        for (int i = 0; i < 4; ++i)
            renderInfo.mShaderParameters[i] = mShaderParameters[i];
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

bool cSprite::SetProperty(unsigned int PropertyFlags, const cPropertyValues& Value)
{
	if (!CheckIfChangableProperty(PropertyFlags))
		return false;
	switch (PropertyFlags)
	{
	case Property_ShaderParam0: setShaderParam(0, Value.ToFloat()); return true;
	case Property_ShaderParam1: setShaderParam(1, Value.ToFloat()); return true;
	case Property_ShaderParam2: setShaderParam(2, Value.ToFloat()); return true;
	case Property_ShaderParam3: setShaderParam(3, Value.ToFloat()); return true;
	}
	return cSpriteBase::SetProperty(PropertyFlags, Value);
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
	case Property_ShaderParam0: PropertyValues = getShaderParam(0); return true;
	case Property_ShaderParam1: PropertyValues = getShaderParam(1); return true;
	case Property_ShaderParam2: PropertyValues = getShaderParam(2); return true;
	case Property_ShaderParam3: PropertyValues = getShaderParam(3); return true;
	default:
		return cSpriteBase::GetProperty(PropertyFlags, PropertyValues);
	}
}

void cSprite::setShaderParam(int index, float value)
{
	ASSERT(index >= 0 && index < 4);
	unsigned int flag = Property_ShaderParam0 << index;
	if (!CheckIfChangableProperty(flag))
		return;
	if (mShaderParameters[index] == value)
		return;
	mShaderParameters[index] = value;
	PropertiesChanged(flag);
}

float cSprite::getShaderParam(int index) const
{
	return mShaderParameters[index];
}

int cSprite::shaderParamIndex(std::string_view name) const
{
	if (!mShader)
	{
		throw std::runtime_error("No shader set for this renderable.");
	}
	return mShader->parameterIndex(name);
}

void cSprite::setShader(std::shared_ptr<cPixelShader> Shader)
{
	if (!CheckIfChangableProperty(Property_Shader))
		return;
	if (mShader == Shader)
		return;
	mShader = Shader;
	PropertiesChanged(Property_Shader);
}

void cSprite::setShader(const std::string& shaderId)
{
	auto shader = theShaderManager->pixelShader(shaderId);
	if (!shader)
	{
		MainLog->Log("Warning: Shader '{}' not found.", shaderId);
	}
	setShader(std::move(shader));
}


tIntrusivePtr<cPixieObjectAnimator> blendShaderParam(
	cSprite& sprite,
	int paramOffset,
	float targetValue,
	int blendTime)
{
	std::print("blendShaderParam called with paramOffset: {}, targetValue: {}, blendTime: {}\n", paramOffset, targetValue, blendTime);
	ASSERT(paramOffset >= 0 && paramOffset <= 3);
	return cGeneralPixieObjectBlender::BlendObject(sprite, cPixieObject::cPropertyValues(targetValue), cPixieObject::Property_ShaderParam0 << paramOffset, blendTime);
}

tIntrusivePtr<cPixieObjectAnimator> blendShaderParam(
	cSprite& sprite,
	std::string_view paramID,
	float targetValue,
	int blendTime)
{
	auto index = sprite.shaderParamIndex(paramID);
	if (index < 0)
	{
		ASSERTFALSE("Invalid shader parameter ID");
		return nullptr;
	}
	return cGeneralPixieObjectBlender::BlendObject(sprite, targetValue, cPixieObject::Property_ShaderParam0 << index, blendTime);
}
