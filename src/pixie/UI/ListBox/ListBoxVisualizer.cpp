#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
size_t cListBox::cVisualizer::GetItemIndexAt(cPoint Position) const
{
	return GetVisualizable().Vis_GetListDirection()==eListDirection::Vertical?
		mItemHandler->GetItemIndexAt(Position.y):
		mItemHandler->GetItemIndexAt(Position.x);
}

cPoint cListBox::cVisualizer::GetItemSize(size_t ItemIndex) const
{
	return GetVisualizable().Vis_GetListDirection()==eListDirection::Vertical?
		cPoint { GetItemsRect().width(), mItemHandler->GetItemSize(ItemIndex) }:
		cPoint { mItemHandler->GetItemSize(ItemIndex), GetItemsRect().height() };
}

void cListBox::cVisualizer::RedrawItems()
{
	cRect ItemsRect=GetItemsRect();

	auto Direction=GetVisualizable().Vis_GetListDirection();
	int GlobalPosition=GetVisualizable().Vis_GetGlobalPosition();
	size_t FirstItem=mItemHandler->GetItemIndexAt(GlobalPosition);

//	cRect CellRect=ItemsRect;
// 	if(Direction==eListDirection::Vertical)
// 		CellRect.mHeight=ItemSize;
// 	else
// 		CellRect.mWidth=ItemSize;


 	std::vector<cItemData> UpdatedVisibleItems;
 	bool HighlightEncountered=false, SelectionEncountered=false;
	if(FirstItem!=cListBox::npos)
	{
		cPoint ItemPosition=Direction==eListDirection::Vertical?
			cPoint { ItemsRect.left(), ItemsRect.top()-(GlobalPosition-mItemHandler->GetItemPosition(FirstItem)) }:
			cPoint { ItemsRect.left()-(GlobalPosition-mItemHandler->GetItemPosition(FirstItem)), ItemsRect.top() };
		size_t FirstNonVisibleItemIndex=mFirstVisibleItemIndex+mVisibleItems.size();
		for(auto i=FirstItem, iend=mItemHandler->GetNumberOfItems(); i<iend&&ItemPosition.y<=ItemsRect.bottom(); ++i)
		{
			if(i>=mFirstVisibleItemIndex&&i<FirstNonVisibleItemIndex)
			{
				// reuse old ItemData
				UpdatedVisibleItems.emplace_back(std::move(mVisibleItems[i-mFirstVisibleItemIndex]));
			}
			else
			{
				// create new ItemData
				UpdatedVisibleItems.emplace_back(mItemHandler->GetItem(i), nullptr);// CreateCellSprite(i, CellRect.GetSize()));
				auto &Item=UpdatedVisibleItems.back();
				Item.mItem->Show(GetWindow(), ItemsRect, mZOrder_Items);
				if(Item.mCellSprite)
				{
					Item.mCellSprite->SetValidRect(ItemsRect);
					Item.mCellSprite->Show();
				}
			}
			// update (or set) position of Item
			auto &Item=UpdatedVisibleItems.back();
			Item.mItem->SetPosition(ItemPosition);
	// 		if(Item.mCellSprite)
	// 		{
	// 			Item.mCellSprite->SetPosition(ItemPosition);
	// 		}
			int ItemSize=mItemHandler->GetItemSize(i);
			cRect ItemRect(ItemPosition, Direction==eListDirection::Vertical?cPoint { ItemsRect.width(), ItemSize }: cPoint { ItemSize, ItemsRect.height() });
			if(GetVisualizable().GetHighlighedIndex()==i)
			{
				SetHighlightMarkerRect(i, ItemRect);
				HighlightEncountered=true;
			}
			if(GetVisualizable().GetSelectionIndex()==i)
			{
				SetSelectionMarkerRect(i, ItemRect);
				SelectionEncountered=true;
			}
			(Direction==eListDirection::Vertical?ItemPosition.y:ItemPosition.x)+=ItemSize;
		}
	}
	if(!SelectionEncountered)
		ClearSelectionMarker();
	if(!HighlightEncountered)
		ClearHighlightMarker();
 	std::swap(UpdatedVisibleItems, mVisibleItems);
 	mFirstVisibleItemIndex=FirstItem;
	UpdateItemsBackground();
}

bool cListBox::cVisualizer::IsItemVisible(size_t Index) const
{
	int GlobalPosition=GetVisualizable().Vis_GetGlobalPosition();
	int GlobalEndPosition=GlobalPosition+
		(GetVisualizable().Vis_GetListDirection()==cListBox::eListDirection::Vertical?GetItemsRect().height():
			GetItemsRect().width());
	int ItemGlobalPos=mItemHandler->GetItemPosition(Index);
	int ItemEndGlobalPos=ItemGlobalPos+mItemHandler->GetItemSize(Index);
	return ItemEndGlobalPos>=GlobalPosition&&ItemGlobalPos<GlobalEndPosition;
}

void cListBox::cVisualizer::MakeItemVisible(size_t Index)
{
	int ItemGlobalPos=mItemHandler->GetItemPosition(Index);
	int ItemEndGlobalPos=ItemGlobalPos+mItemHandler->GetItemSize(Index);
	//	int ItemMiddlePos=(ItemGlobalPos+ItemEndGlobalPos)/2;
	int GlobalPosition=GetVisualizable().Vis_GetGlobalPosition();
	int GlobalEndPosition=GlobalPosition+
		(GetVisualizable().Vis_GetListDirection()==cListBox::eListDirection::Vertical?GetItemsRect().height():
			GetItemsRect().width());
	if(ItemGlobalPos<GlobalPosition)
	{
		GetVisualizable().SetGlobalPosition(ItemGlobalPos);
	}
	else if(ItemEndGlobalPos>GlobalEndPosition)
	{
		GetVisualizable().SetGlobalPosition(ItemEndGlobalPos-(GlobalEndPosition-GlobalPosition));
	}
}

void cListBox::cVisualizer::InvalidateItems()
{
	mVisibleItems.clear();
}

cListBox::cVisualizer::cItemData::~cItemData()
{
	if(mItem)
		mItem->Hide();
}