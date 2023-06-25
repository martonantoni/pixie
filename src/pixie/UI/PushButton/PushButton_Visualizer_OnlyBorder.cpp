#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/ui/PushButton/PushButton_Visualizer_OnlyBorder.h"

void cPushButtonVisualizer_OnlyBorder::Init(const cPushButton::cInitData &InitData)
{
}

void cPushButtonVisualizer_OnlyBorder::StateChanged()
{
	Redraw();
}

void cPushButtonVisualizer_OnlyBorder::Redraw()
{
	bool MouseInside=GetVisualizable().IsMouseInside(), InFocus=GetVisualizable().IsInFocus(), Pushed=GetVisualizable().IsPushed();
	if(!MouseInside&&!InFocus&&!Pushed)
	{
		if(mBorderSprite)
		{
			mOldTargetAlpha=255;
			BlendObjectAlpha(*mBorderSprite, 255, 100);
			mBorderSprite.reset();
		}
		return;
	}
	if(!mBorderSprite)
	{
		mBorderSprite=std::make_unique<cMultiSprite_1PixRect>("editor_hover", cMultiSprite_1PixRect::eOnlyBorder::Yes);
		mBorderSprite->SetWindow(GetWindow());
		mBorderSprite->SetPlacement(GetPlacement());
		mBorderSprite->SetZOrder(GetBaseZ());
		mBorderSprite->SetAlpha(255);
		mBorderSprite->Show();
	}
	int TargetAlpha=0;
	if(!MouseInside)
	{
		if(InFocus)
			TargetAlpha=128;
		else
			TargetAlpha=255;
	}
	if(mOldTargetAlpha!=TargetAlpha)
	{
		BlendObjectAlpha(*mBorderSprite, TargetAlpha, 100);
		mOldTargetAlpha=TargetAlpha;
	}
}

REGISTER_VISUALIZER(cPushButton, "only_border", cPushButtonVisualizer_OnlyBorder);

