#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/ui/PushButton/PushButton_Visualizer_Standard.h"

template<class T>
void tStandardPushButtonVisualizer<T>::Init(const cPushButton::cInitData &InitData)
{
	mTextStyle=InitData.mTextStyle;
	mTextureBaseName=InitData.mTextureBaseName;
	if(mTextureBaseName.empty())
		mTextureBaseName="ui_button";
	mUseTextRenderer=InitData.mConfig.GetBool("use_text_renderer", mUseTextRenderer);
}

template<class T>
void tStandardPushButtonVisualizer<T>::Init()
{
	mInitDone=true;

	cRect ValidRect=GetVisualizable().GetValidRect();
	bool UseClipping=GetVisualizable().GetUseClipping();

	mDepthSprite=std::make_unique<T>();
	mDepthSprite->Init(mWindow, mTextureBaseName+"_depth");
	mDepthSprite->SetPlacement(GetPlacement());
	mDepthSprite->SetZOrder(mBaseZ);
	if(UseClipping) 
		mDepthSprite->SetValidRect(ValidRect);
	mDepthSprite->SetRGBColor(0u);

	mBaseSprite=std::make_unique<T>();
	mBaseSprite->Init(mWindow, mTextureBaseName+"_base");
	mBaseSprite->SetPlacement(GetPlacement());
	if(UseClipping) 
		mBaseSprite->SetValidRect(ValidRect);
	mBaseSprite->SetZOrder(mBaseZ+1);

	mHoverSprite=std::make_unique<T>();
	mHoverSprite->Init(mWindow, mTextureBaseName+"_hover");
	mHoverSprite->SetPlacement(GetPlacement());
	mHoverSprite->SetZOrder(mBaseZ+2);
	if(UseClipping) 
		mHoverSprite->SetValidRect(ValidRect);
	mHoverSprite->SetAlpha(255);

	mPushedSprite=std::make_unique<T>();
	mPushedSprite->Init(mWindow, mTextureBaseName+"_pushed");
	mPushedSprite->SetPlacement(GetPlacement());
	mPushedSprite->SetZOrder(mBaseZ+3);
	if(UseClipping) 
		mPushedSprite->SetValidRect(ValidRect);
	mPushedSprite->SetAlpha(255);

	CreateTextSprite();
}

template<class T>
void tStandardPushButtonVisualizer<T>::CreateTextSprite()
{
	cRect ValidRect=GetVisualizable().GetValidRect();
	bool UseClipping=GetVisualizable().GetUseClipping();
	if(mUseTextRenderer)
	{
		cTextRenderer TextRenderer(mTextStyle);
		TextRenderer.SetDefaultColor(GetTextColor());
		auto Placement=GetPlacement();
		TextRenderer.SetMaxWidth(Placement.Width());
		mLastText=GetVisualizable().GetText();
		mTextSprite_TR=std::make_unique<cSimpleMultiSprite>(TextRenderer.RenderText(mLastText), cSimpleMultiSprite::eBasePosition::Zero);
		mTextSprite_TR->SetWindow(mWindow);
		auto VerticalAlignedPlacement=Placement.GetAlignedRect(mTextSprite_TR->GetRect(), eHorizontalAlign::Left, eVerticalAlign::Center);
		mTextSprite_TR->SetPlacement(VerticalAlignedPlacement);

		if(UseClipping)
			mTextSprite_TR->SetValidRect(ValidRect);
		mTextSprite_TR->SetZOrder(mBaseZ+4);
	}
	else
	{
		mTextSprite=std::make_unique<cTextSprite>();
		mTextSprite->SetWindow(mWindow);
		mTextSprite->SetFont(mTextStyle);
		mTextSprite->SetText(GetVisualizable().GetText());
		mTextSprite->SetRGBColor(GetTextColor());
		mTextSprite->SetPlacement(GetPlacement());
		mTextSprite->SetAlignment(eHorizontalAlign::Center, eVerticalAlign::Center);
		if(UseClipping)
			mTextSprite->SetValidRect(ValidRect);
		mTextSprite->SetZOrder(mBaseZ+4);
	}
}

template<class T>
cColor tStandardPushButtonVisualizer<T>::GetTextColor() const
{
	return cColor {
		!GetVisualizable().IsEnabled()?mTextStyle+"_disabled":GetVisualizable().IsMouseInside()
		||GetVisualizable().IsPushed()?mTextStyle+"_highlight":mTextStyle+"_text" };
}

template<class T>
void tStandardPushButtonVisualizer<T>::StateChanged()
{
	RedrawNeeded();
}

template<class T>
void tStandardPushButtonVisualizer<T>::Redraw()
{
	if(!mInitDone)
		Init();

	if(mLastVisible!=GetVisualizable().IsVisible())
	{
		mLastVisible=GetVisualizable().IsVisible();
		if(!mLastVisible)
		{
			mDepthSprite->Hide();
			mBaseSprite->Hide();
			mHoverSprite->Hide();
			mPushedSprite->Hide();
			if(mUseTextRenderer)
				mTextSprite_TR->Hide();
			else
				mTextSprite->Hide();
		}
		else
		{
			mDepthSprite->Show();
			mBaseSprite->Show();
			mHoverSprite->Show();
			mPushedSprite->Show();
			if(mUseTextRenderer)
				mTextSprite_TR->Show();
			else
				mTextSprite->Show();
		}
	}

	mDepthSprite->SetWindow(mWindow);
	mBaseSprite->SetWindow(mWindow);
	mHoverSprite->SetWindow(mWindow);
	mPushedSprite->SetWindow(mWindow);
	if(mUseTextRenderer)
		mTextSprite_TR->SetWindow(mWindow);
	else
		mTextSprite->SetWindow(mWindow);

	mDepthSprite->SetPlacement(GetPlacement());
	mBaseSprite->SetPlacement(GetPlacement());
	mHoverSprite->SetPlacement(GetPlacement());
	mPushedSprite->SetPlacement(GetPlacement());
	if(mUseTextRenderer)
	{
		if(mLastText!=GetVisualizable().GetText())
		{
			CreateTextSprite();
			if(mLastVisible)
				mTextSprite_TR->Show();
		}
	}
	else
	{
		mTextSprite->SetPlacement(GetPlacement());
		mTextSprite->SetText(GetVisualizable().GetText());
	}

	int TargetHoverAlpha=255,TargetPushedAlpha=255;
	std::string TargetTextColor=mTextStyle+"_text";
	if(GetVisualizable().IsPushed())
	{
		TargetPushedAlpha=0;
	}
	else if(GetVisualizable().IsMouseInside())
	{
		TargetHoverAlpha=0;
	}
	if(!mUseTextRenderer)
	{
		::BlendObjectRGBColor_NoKeepAlive(*mTextSprite, GetTextColor(), 100);
	}
	::BlendObjectAlpha_NoKeepAlive(*mHoverSprite, TargetHoverAlpha, 100);
	::BlendObjectAlpha_NoKeepAlive(*mPushedSprite,TargetPushedAlpha,200);
}

typedef tStandardPushButtonVisualizer<cMultiSprite_Horiz3> cStandardPushButtonVisualizer;
typedef tStandardPushButtonVisualizer<cMultiSprite_Simple9Patch> c9PatchStandardPushButtonVisualizer;

REGISTER_VISUALIZER(cPushButton, "standard", cStandardPushButtonVisualizer);
REGISTER_VISUALIZER(cPushButton, "standard_9patch", c9PatchStandardPushButtonVisualizer);

