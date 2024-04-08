#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
INIT_VISUALIZER_IMP(cTextField);

void cTextField::cInitData::setConfig(std::shared_ptr<cConfig> Config)
{
	tPixieSimpleInitData<cMouseTarget>::setConfig(Config);
	mTitle=Config->get<std::string>("title", std::string());
	mTextStyle=Config->get<std::string>("text_style", mTextStyle);
}

void cTextField::Init(const cInitData &InitData)
{
	cDialogItem::Init(InitData);
	InitVisualizer(InitData, InitData.mVisualizer);
	if(!InitData.mTitle.empty())
		SetText(InitData.mTitle);
}

void cTextField::DialogItemStateChanged()
{
	mVisualizer->StateChanged();
}
