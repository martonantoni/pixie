#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/ui/Visualizers/vis_Standard.h"

void cVisualizer_Standard::Init(const cStandardWindow::cInitData &InitData)
{
	mBackgroundTextureName=InitData.mConfig->GetString("background_texture", std::string());
}

void cVisualizer_Standard::Redraw()
{
	if(!mBorderSprite)
	{
		mBorderSprite=std::make_unique<cMultiSprite_Simple9Patch>();
		if(!mBackgroundTextureName.empty())
		{
			mBorderSprite->OverrideTexture(cMultiSprite_Simple9Patch::Sprite_Middle, theTextureManager.GetTexture(mBackgroundTextureName));
			mBorderSprite->SetCenterSpriteMode(cMultiSprite_Simple9Patch::eCenterSpriteMode::Entire);
		}
		mBorderSprite->Init(mWindow, "simple_window");
		mBorderSprite->SetPosition(0, 0);
		mBorderSprite->SetZOrder(1);
		mBorderSprite->Show();
	}
	cRect Placement=GetPlacement();
	mBorderSprite->SetSize(Placement.mWidth, Placement.mHeight);
}

cVisualizer_Standard::~cVisualizer_Standard()
{
}

REGISTER_VISUALIZER(cStandardWindow,"standard",cVisualizer_Standard);