#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/ui/TextField/TextField_Visualizer_Standard.h"

void cStandardTextFieldVisualizer::Init(const cTextField::cInitData &InitData)
{
	mTextStyle=InitData.mTextStyle;
	RedrawNeeded();
}

void cStandardTextFieldVisualizer::StateChanged()
{
	RedrawNeeded();
}

void cStandardTextFieldVisualizer::Redraw()
{
	cRect ValidRect=GetVisualizable().GetValidRect();
	bool UseClipping=GetVisualizable().GetUseClipping();
	cRect Rect=GetPlacement();
	cTextRenderer TextRenderer(mTextStyle);
	TextRenderer.setMaxWidth(Rect.width());
	TextRenderer.setDefaultColor("textfield");
	mTextSprite=std::make_unique<cSimpleMultiSprite>(TextRenderer.render(GetVisualizable().GetText()), cSimpleMultiSprite::eBasePosition::Zero);
	mTextSprite->SetWindow(GetWindow());
	mTextSprite->SetPosition(Rect.position());
	mTextSprite->SetZOrder(mBaseZ);
	mTextSprite->Show();
}

REGISTER_VISUALIZER(cTextField, "standard", cStandardTextFieldVisualizer);

