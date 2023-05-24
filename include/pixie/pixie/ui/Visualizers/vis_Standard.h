#pragma once

class cVisualizer_Standard: public cStandardWindow::cVisualizer
{
	std::unique_ptr<cMultiSprite_Simple9Patch> mBorderSprite;
	std::string mBackgroundTextureName;

public:
	virtual ~cVisualizer_Standard();
	virtual void Init(const cStandardWindow::cInitData &InitData) override;
	virtual void Redraw() override;
};