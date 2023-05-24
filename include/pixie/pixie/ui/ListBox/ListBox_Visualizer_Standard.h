#pragma once

class cStandardListBoxVisualizer: public cListBox::cVisualizer
{
	tIntrusivePtr<cTexture> mBackgroundTexture;
	std::unique_ptr<cSimpleMultiSprite> mBackgroundSprites;
	size_t mHighlightIndex=~0;
	std::unique_ptr<cSpriteBase> mHighlightSprite;
	size_t mSelectionIndex=~0;
	std::unique_ptr<cSpriteBase> mSelectionSprite;
	std::unique_ptr<cSpriteBase> CreateMarkerSprite(const cRect &Rect);
	virtual void UpdateItemsBackground() override;
	virtual void SetSelectionMarkerRect(size_t ItemIndex, const cRect &Rect) override;
	virtual void SetHighlightMarkerRect(size_t ItemIndex, const cRect &Rect) override;
	virtual void ClearSelectionMarker() override;
	virtual void ClearHighlightMarker() override;
public:
	cStandardListBoxVisualizer();
	~cStandardListBoxVisualizer();
	void Init(const cConfig &Config);
	virtual void StateChanged() override;
	virtual void Redraw() override;
	virtual cRect GetItemsRect() const override;
};
