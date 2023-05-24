#pragma once

class cFont2;

// PositionOffset is not changeable for TextSprites
//   (uses it internally for handling the alignment)

class cTextSprite: public cSprite
{
	cFont2 *mFont=nullptr;
	eHorizontalAlign mHorizontalAlign=eHorizontalAlign::Left;
	eVerticalAlign mVeritcalAlign=eVerticalAlign::Center;
	std::string mText;
	cRegisteredID mUpdateTextureID;
    bool mIsResizable = false;
	void UpdateTexture();
	void ConstructorCommon();
	void NeedTextureUpdate();
	void HandleAlignment();
protected:
	virtual void PropertiesChanged(unsigned int Properties) override;
	virtual cSpriteRenderInfo GetRenderInfo() const override;
	virtual ~cTextSprite()=default;
public:
	cTextSprite();   
	void SetFont(cFont2 *Font);
	void SetFont(const std::string &FontName);
	void SetAlignment(eHorizontalAlign Horizontal, eVerticalAlign Vertical);
	void SetText(const std::string &Text);
    void SetSizeToTextureSize();
    void SetResizable(bool Resizable = true);

	virtual bool SetStringProperty(unsigned int PropertyFlags, const std::string &Value) override;
};

USE_DROP_INSTEAD_DELETE_PARENT(cTextSprite, cSprite)

