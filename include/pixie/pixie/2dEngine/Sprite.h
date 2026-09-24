#pragma once

class cSprite: public cSpriteBase, public tIntrusiveListItemBase<cSprite>
{
	friend class cSpriteRenderer;
protected:
	tIntrusivePtr<cTexture> mTexture;
	std::shared_ptr<cPixelShader> mShader;
	float mShaderParameters[4] = { 0, 0, 0, 0 };
	cSpriteRenderInfo::eBlendingMode mBlendingMode=cSpriteRenderInfo::Blend_Normal;
	virtual ~cSprite()=default; // protected. use Drop
public:
	cSprite()=default; 
	void SetBlendingMode(cSpriteRenderInfo::eBlendingMode BlendingMode);
	void SetTexture(const std::string &TextureName) { SetTexture(theTextureManager.GetTexture(TextureName)); }
	void SetTexture(tIntrusivePtr<cTexture> Texture);
	void SetTextureAndSize(tIntrusivePtr<cTexture> Texture);
	void SetTextureAndSize(const std::string &TextureName) { SetTextureAndSize(theTextureManager.GetTexture(TextureName)); }
	const tIntrusivePtr<cTexture> &GetTexture() const { return mTexture; }
	cSpriteRenderInfo::eBlendingMode GetBlendingMode() const { return mBlendingMode; }
	virtual cSpriteRenderInfo GetRenderInfo() const override;
	virtual cPoint GetPrefferedSize() const override { return mTexture?mTexture->GetSize():cPoint {1, 1}; }
	virtual bool SetStringProperty(unsigned int PropertyFlags, const std::string &Value) override;
	virtual bool GetProperty(unsigned int PropertyFlags, OUT cPropertyValues &Value) const override;
	virtual bool SetProperty(unsigned int PropertyFlags, const cPropertyValues& Value) override;

	virtual void updateTextures() override;

	void setShader(std::shared_ptr<cPixelShader> Shader);
	void setShader(const std::string& shaderId);
	std::shared_ptr<cPixelShader> getShader() const { return mShader; }
	void setShaderParam(int index, float value);
	float getShaderParam(int index) const;
	int shaderParamIndex(std::string_view name) const; // throws if not found
};

USE_DROP_INSTEAD_DELETE_PARENT(cSprite, cSpriteBase)


tIntrusivePtr<cPixieObjectAnimator> blendShaderParam(
	cSprite& sprite, int paramOffset, float targetValue, int blendTime);

tIntrusivePtr<cPixieObjectAnimator> blendShaderParam(
	cSprite& sprite, std::string_view paramID, float targetValue, int blendTime);


