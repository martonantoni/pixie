#pragma once

class cSprite: public cSpriteBase, public tIntrusiveListItemBase<cSprite>
{
	friend class cSpriteRenderer;
protected:
	std::array<tIntrusivePtr<cTexture>, 4> mTextures;
	std::shared_ptr<cPixelShader> mShader;
	float mShaderParameters[4] = { 0, 0, 0, 0 };
	cSpriteRenderInfo::eBlendingMode mBlendingMode=cSpriteRenderInfo::Blend_Normal;
	virtual ~cSprite()=default; // protected. use Drop
public:
	cSprite()=default; 
	void SetBlendingMode(cSpriteRenderInfo::eBlendingMode BlendingMode);
    void setTexture(int index, tIntrusivePtr<cTexture> Texture);
    void setTexture(std::string_view slotID, tIntrusivePtr<cTexture> Texture);
	void setTexture(const std::string &TextureName) { setTexture(0, theTextureManager.getTexture(TextureName)); }
    void setTexture(tIntrusivePtr<cTexture> Texture) { setTexture(0, std::move(Texture)); }
	void setTextureAndSize(tIntrusivePtr<cTexture> Texture);
	void setTextureAndSize(const std::string &TextureName) { setTextureAndSize(theTextureManager.getTexture(TextureName)); }
	const tIntrusivePtr<cTexture> &getTexture() const { return mTextures[0]; }
	cSpriteRenderInfo::eBlendingMode GetBlendingMode() const { return mBlendingMode; }
	virtual cSpriteRenderInfo GetRenderInfo() const override;
	virtual cPoint GetPrefferedSize() const override { return mTextures[0]?mTextures[0]->GetSize():cPoint {1, 1}; }
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


