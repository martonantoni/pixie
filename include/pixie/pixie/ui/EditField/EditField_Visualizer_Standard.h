#pragma once

class cStandardEditFieldVisualizer: public cEditField::cVisualizer
{
	std::string mTextStyle;
	std::unique_ptr<cSimpleMultiSprite> mTextSprite;
	std::unique_ptr<cSprite> mCursorSprite;
public:
	virtual ~cStandardEditFieldVisualizer()=default;
	virtual void Init(const cEditField::cInitData &InitData) override;
	virtual void StateChanged() override;
	virtual void Redraw() override;
};
