#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
INIT_VISUALIZER_IMP(cTextField);

cTextField::cInitData::cInitData(const tIntrusivePtr<cConfig> &Config): tPixieSimpleInitData<cMouseTarget>(Config)
{
	mTitle=Config->GetString("title", std::string());
	mTextStyle=Config->GetString("text_style", mTextStyle);
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
