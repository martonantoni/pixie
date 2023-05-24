#pragma once

template<class T>
class tStandardPushButtonVisualizer: public cPushButton::cVisualizer
{
	std::string mTextStyle;
	std::string mTextureBaseName;
	bool mInitDone=false;
	bool mLastVisible=false;
	bool mUseTextRenderer=false;
	std::string mLastText;
	std::unique_ptr<T> mDepthSprite;
	std::unique_ptr<T> mBaseSprite;
	std::unique_ptr<T> mHoverSprite;
	std::unique_ptr<T> mPushedSprite;
	std::unique_ptr<cTextSprite> mTextSprite;
	std::unique_ptr<cSpriteBase> mTextSprite_TR;
	cColor GetTextColor() const;
	void CreateTextSprite();
	void Init();
public:
	virtual ~tStandardPushButtonVisualizer() {}
	virtual void Init(const cPushButton::cInitData &InitData) override;
	virtual void StateChanged() override;
	virtual void Redraw() override;
};
