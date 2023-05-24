#include "StdAfx.h"

#include "TextField_Visualizer_Standard.h"

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
	TextRenderer.SetMaxWidth(Rect.Width());
	TextRenderer.SetDefaultColor("textfield");
	mTextSprite=std::make_unique<cSimpleMultiSprite>(TextRenderer.RenderText(GetVisualizable().GetText()), cSimpleMultiSprite::eBasePosition::Zero);
	mTextSprite->SetWindow(GetWindow());
	mTextSprite->SetPosition(Rect.GetPosition());
	mTextSprite->SetZOrder(mBaseZ);
	mTextSprite->Show();
}

REGISTER_VISUALIZER(cTextField, "standard", cStandardTextFieldVisualizer);

