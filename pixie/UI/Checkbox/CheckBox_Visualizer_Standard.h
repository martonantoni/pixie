#pragma once

class cStandardCheckBoxVisualizer: public cCheckBox::cVisualizer
{
	cSprite *mUncheckedSprite;
	cSprite *mCheckMarkSprite;
	cTextSprite *mTextSprite;
public:
	virtual ~cStandardCheckBoxVisualizer();
	virtual void StateChanged() override;
	virtual void Init(const cCheckBox::cInitData &InitData) override;
	virtual void Redraw() override;
};