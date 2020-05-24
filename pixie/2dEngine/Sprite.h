#pragma once

class cSprite: public cSpriteBase, public tIntrusiveListItemBase<cSprite>
{
	friend class cSpriteRenderer;
protected:
	mutable tIntrusivePtr<cTexture> mClippedTexture;
	tIntrusivePtr<cTexture> mTexture;
	cSpriteRenderInfo::eBlendingMode mBlendingMode=cSpriteRenderInfo::Blend_Normal;
    void CopyProperties(const cSprite& source);
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
    virtual std::unique_ptr<cSpriteBase> Clone() const override;
};

USE_DROP_INSTEAD_DELETE_PARENT(cSprite, cSpriteBase)

