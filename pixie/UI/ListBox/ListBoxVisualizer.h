#pragma once

class cListBox::cVisualizer: public tVisualizerBase<cListBox>
{
	struct cItemData
	{
		std::shared_ptr<cListBoxItem> mItem;
		std::unique_ptr<cSpriteBase> mCellSprite;
		std::unique_ptr<cSpriteBase> mHighlightSprite;
		std::unique_ptr<cSpriteBase> mSelectionSprite;

		~cItemData();
		cItemData()=default;
		cItemData(std::shared_ptr<cListBoxItem> Item, std::unique_ptr<cSpriteBase> CellSprite): mItem(std::move(Item)), mCellSprite(std::move(CellSprite)) {}
		cItemData(const cItemData &)=delete;
		cItemData(cItemData &&)=default;
	};
	std::vector<cItemData> mVisibleItems;
	size_t mFirstVisibleItemIndex=0; // index of mVisibleItems[0] within the entire list
	int mZOrder_Items=600;
protected:
	std::shared_ptr<cListBoxItemHandler> mItemHandler;
	void RedrawItems();
	cPoint GetItemSize(size_t ItemIndex) const;
//	virtual std::unique_ptr<cSpriteBase> CreateCellSprite(size_t ItemIndex, cPoint Size) const { return nullptr; }
	virtual void SetSelectionMarkerRect(size_t ItemIndex, const cRect &Rect) {}
	virtual void SetHighlightMarkerRect(size_t ItemIndex, const cRect &Rect) {}
	virtual void ClearSelectionMarker() {}
	virtual void ClearHighlightMarker() {}
	virtual void UpdateItemsBackground() {}
public:
	virtual void StateChanged()=0;
	virtual cRect GetItemsRect() const=0;
	void InvalidateItems();
	bool IsItemVisible(size_t Index) const;
	void MakeItemVisible(size_t Index);

	size_t GetItemIndexAt(cPoint Position) const;
	void SetItemHandler(std::shared_ptr<cListBoxItemHandler> ItemHandler) { mItemHandler=std::move(ItemHandler); }
};
