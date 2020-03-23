#pragma once

class cPushButtonVisualizer_OnlyBorder: public cPushButton::cVisualizer
{
	std::unique_ptr<cSpriteBase> mBorderSprite;
	int mOldTargetAlpha=-1;
public:
	virtual void Init(const cPushButton::cInitData &InitData) override;
	virtual void StateChanged() override;
	virtual void Redraw() override;
};
