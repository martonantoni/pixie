#include "StdAfx.h"

#include "CheckBox_Visualizer_Standard.h"

cStandardCheckBoxVisualizer::~cStandardCheckBoxVisualizer()
{
}

void cStandardCheckBoxVisualizer::StateChanged()
{
	RedrawNeeded();
}

void cStandardCheckBoxVisualizer::Redraw()
{
	int TargetCheckMarkAlpha=GetVisualizable().IsChecked()?0:255;
	BlendObjectAlpha(*mCheckMarkSprite,TargetCheckMarkAlpha,100);
	mTextSprite->SetText(GetVisualizable().GetText());
}

void cStandardCheckBoxVisualizer::Init(const cCheckBox::cInitData &InitData)
{
	mUncheckedSprite=new cSprite;
	mUncheckedSprite->SetWindow(GetWindow());
	tIntrusivePtr<cTexture> UncheckedTexture(theTextureManager.GetTexture("ui_checkbox_w_arrow_mark"));
	mUncheckedSprite->SetZOrder(100);
	mUncheckedSprite->SetTexture(UncheckedTexture);
	mUncheckedSprite->SetPosition(GetPlacement().GetPosition());
	mUncheckedSprite->SetSize(UncheckedTexture->GetSurfaceWidth(),UncheckedTexture->GetSurfaceHeight());
	mUncheckedSprite->SetBlendingMode(cSpriteRenderInfo::Blend_Normal);
	mUncheckedSprite->Show();

	mCheckMarkSprite=new cSprite;
	mCheckMarkSprite->SetWindow(GetWindow());
	tIntrusivePtr<cTexture> CheckMarkTexture(theTextureManager.GetTexture("ui_checkbox_arrow_mark"));
	mCheckMarkSprite->SetZOrder(101);
	mCheckMarkSprite->SetTexture(CheckMarkTexture);
	mCheckMarkSprite->SetPosition(GetPlacement().GetPosition());
	mCheckMarkSprite->SetSize(CheckMarkTexture->GetSurfaceWidth(),CheckMarkTexture->GetSurfaceHeight());
	mCheckMarkSprite->SetBlendingMode(cSpriteRenderInfo::Blend_Normal);
	mCheckMarkSprite->SetAlpha(GetVisualizable().IsChecked()?0:255);
	mCheckMarkSprite->Show();

	mTextSprite=new cTextSprite;
	mTextSprite->SetWindow(GetWindow());
	cRect TextRect(UncheckedTexture->GetSurfaceWidth()+4,0,mWindow->GetPlacement().mWidth,mWindow->GetPlacement().mHeight);
	TextRect.Move(GetPlacement().GetPosition());
	mTextSprite->SetFont("default");
	mTextSprite->SetRect(TextRect);
	mTextSprite->SetText(GetVisualizable().GetText());
	mTextSprite->SetARGBColor(D3DCOLOR_ARGB(0xff,0xe0,0xe0,0xe0));
	mTextSprite->SetAlignment(eHorizontalAlign::Left, eVerticalAlign::Center);
	mTextSprite->SetZOrder(11);
	mTextSprite->Show();
}

REGISTER_VISUALIZER(cCheckBox,"standard",cStandardCheckBoxVisualizer);
