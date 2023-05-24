#pragma once

class cStandardTextFieldVisualizer: public cTextField::cVisualizer
{
	std::string mTextStyle;
	std::unique_ptr<cSimpleMultiSprite> mTextSprite;
public:
	virtual ~cStandardTextFieldVisualizer()=default;
	virtual void Init(const cTextField::cInitData &InitData) override;
	virtual void StateChanged() override;
	virtual void Redraw() override;
};
