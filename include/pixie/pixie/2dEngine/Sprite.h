#pragma once

class cSprite: public cSpriteBase, public tIntrusiveListItemBase<cSprite>
{
	friend class cSpriteRenderer;
protected:
    cTextureBindings mTextureBindings;
	std::shared_ptr<cPixelShader> mShader;
	float mShaderParameters[4] = { 0, 0, 0, 0 };
	cSpriteRenderInfo::eBlendingMode mBlendingMode=cSpriteRenderInfo::Blend_Normal;
    cRegisteredID mShaderReloadedListenerID;
	virtual ~cSprite()=default; // protected. use Drop
	std::bitset<4> retrieveTexturesFromShader();
public:
	cSprite()=default; 
	void SetBlendingMode(cSpriteRenderInfo::eBlendingMode BlendingMode);
    void setTexture(int index, tIntrusivePtr<cTexture> Texture);
    void setTexture(std::string_view slotID, tIntrusivePtr<cTexture> Texture);
	void setTexture(const std::string &TextureName) { setTexture(0, theTextureManager.getTexture(TextureName)); }
    void setTexture(tIntrusivePtr<cTexture> Texture) { setTexture(0, std::move(Texture)); }
	void setTextureAndSize(tIntrusivePtr<cTexture> Texture);
	void setTextureAndSize(const std::string &TextureName) { setTextureAndSize(theTextureManager.getTexture(TextureName)); }
	cSpriteRenderInfo::eBlendingMode GetBlendingMode() const { return mBlendingMode; }
	virtual cSpriteRenderInfo GetRenderInfo() const override;
	virtual cPoint GetPrefferedSize() const override;
	virtual bool SetStringProperty(unsigned int PropertyFlags, const std::string &Value) override;
	virtual bool GetProperty(unsigned int PropertyFlags, OUT cPropertyValues &Value) const override;
	virtual bool SetProperty(unsigned int PropertyFlags, const cPropertyValues& Value) override;

	virtual void updateTextures() override;

	void setShader(std::shared_ptr<cPixelShader> Shader);
	void setShader(const std::string& shaderId);
	std::shared_ptr<cPixelShader> getShader() const { return mShader; }
	void setShaderParam(int index, float value);
    void setShaderParam(std::string_view paramID, float value) { setShaderParam(shaderParamIndex(paramID), value); }
	float getShaderParam(int index) const;
	int shaderParamIndex(std::string_view name) const; // throws if not found
};

USE_DROP_INSTEAD_DELETE_PARENT(cSprite, cSpriteBase)


tIntrusivePtr<cPixieObjectAnimator> blendShaderParam(
	cSprite& sprite, int paramOffset, float targetValue, int blendTime);

tIntrusivePtr<cPixieObjectAnimator> blendShaderParam(
	cSprite& sprite, std::string_view paramID, float targetValue, int blendTime);


