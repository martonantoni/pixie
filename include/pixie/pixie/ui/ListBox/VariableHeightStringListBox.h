#pragma once

class cVariableHeightStringListBoxItem: public cListBoxItem
{
	std::shared_ptr<cTextRenderer> mTextRenderer;
	std::string mText;
	std::unique_ptr<cSimpleMultiSprite> mSprite;
	int mHeight=0;
	int mGlobalPosition=0;
public:
	cVariableHeightStringListBoxItem(std::shared_ptr<cTextRenderer> TextRenderer, const std::string &Text);
	void Render(int MaxWidth);
	virtual void SetPosition(cPoint Position) override;
	virtual void Show(cPixieWindow *Window, const cRect &ValidRect, int ZOrder) override;
	virtual void Hide() override;
	const std::string &GetText() const { return mText; }
	int GetHeight() const { return mHeight; }
	bool IsRendered() const { return !!mSprite; }
	int GetGlobalPosition() const { return mGlobalPosition; }
	void SetGlobalPosition(int GlobalPosition) { mGlobalPosition=GlobalPosition; }
};

class cVariableHeightStringListBoxItemHandler: public cListBoxItemHandler
{
	std::shared_ptr<cTextRenderer> mTextRenderer;
	std::vector<std::shared_ptr<cVariableHeightStringListBoxItem>> mItems;
	bool mIsItemPropsValid=false;
	void Init();
	void UpdateItems();
public:
	cVariableHeightStringListBoxItemHandler();
	cVariableHeightStringListBoxItemHandler(const std::vector<std::string> &Items);

	virtual size_t GetItemIndexAt(int Position) override;
	virtual int GetItemSize(size_t ItemIndex) const override;
	virtual int GetItemPosition(size_t ItemIndex) const override;
	virtual std::shared_ptr<cListBoxItem> GetItem(size_t Index) override;
	virtual size_t GetNumberOfItems() const override;

	void InsertString(size_t Position, const std::string &Text);
	void AddString(const std::string &Text);
	void setTabStops(std::vector<int> &&TabStops);
};

