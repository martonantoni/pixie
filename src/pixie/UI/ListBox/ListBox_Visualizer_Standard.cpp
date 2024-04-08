#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/ui/ListBox/ListBox_Visualizer_Standard.h"

cStandardListBoxVisualizer::cStandardListBoxVisualizer()
{
	mBackgroundTexture=theTextureManager.GetTexture("listbox_background");
}

cStandardListBoxVisualizer::~cStandardListBoxVisualizer()
{
}

void cStandardListBoxVisualizer::Init(const cConfig2 &Config)
{
}

void cStandardListBoxVisualizer::StateChanged()
{
	RedrawNeeded();
}

void cStandardListBoxVisualizer::Redraw()
{
	RedrawItems();
}

cRect cStandardListBoxVisualizer::GetItemsRect() const
{
	cRect Rect=GetVisualizable().GetPlacement();
	Rect.SetPosition(Rect.GetPosition()+cPoint { 1,1 });
	Rect.SetSize(Rect.GetSize()-cPoint { 2,2 });
	return Rect;
}

std::unique_ptr<cSpriteBase> cStandardListBoxVisualizer::CreateMarkerSprite(const cRect &Rect)
{
	auto CellSprite=std::make_unique<cMultiSprite_Simple9Patch>();
	CellSprite->Init(GetWindow(), "ui_lb_item_bg");
	CellSprite->SetSize(Rect.GetSize());
	CellSprite->SetValidRect(GetItemsRect());
	CellSprite->SetZOrder(301);
	CellSprite->Show();
	return CellSprite;
}

void cStandardListBoxVisualizer::SetSelectionMarkerRect(size_t ItemIndex, const cRect &Rect)
{
	if(ItemIndex!=mSelectionIndex||!mSelectionSprite)
	{
		mSelectionIndex=ItemIndex;
		mSelectionSprite=CreateMarkerSprite(Rect);
		mSelectionSprite->SetRGBColor("listbox_selection");
	}
	mSelectionSprite->SetPosition(Rect.GetPosition());
}

void cStandardListBoxVisualizer::SetHighlightMarkerRect(size_t ItemIndex, const cRect &Rect)
{
	if(ItemIndex!=mHighlightIndex||!mHighlightSprite)
	{
		mSelectionIndex=ItemIndex;
		mHighlightSprite=CreateMarkerSprite(Rect);
		mHighlightSprite->SetRGBColor("listbox_highlight");
	}
	mHighlightSprite->SetPosition(Rect.GetPosition());
}

void cStandardListBoxVisualizer::ClearSelectionMarker()
{
	mSelectionSprite.reset();
	mSelectionIndex=~0;
}

void cStandardListBoxVisualizer::ClearHighlightMarker()
{
	mHighlightSprite.reset();
	mHighlightIndex=~0;
}


void cStandardListBoxVisualizer::UpdateItemsBackground()
{
	mBackgroundSprites=std::make_unique<cSimpleMultiSprite>(CreateTileFillSprites(mBackgroundTexture, GetItemsRect().GetSize(), GetVisualizable().Vis_GetPosition()));
	mBackgroundSprites->SetPosition(GetItemsRect().GetPosition());
	mBackgroundSprites->SetWindow(GetWindow());
	mBackgroundSprites->SetZOrder(300);
	mBackgroundSprites->Show();
}

REGISTER_VISUALIZER(cListBox, "standard", cStandardListBoxVisualizer);

