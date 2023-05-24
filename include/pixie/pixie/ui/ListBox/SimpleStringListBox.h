#pragma once

class cSimpleStringListBoxItem: public cListBoxItem
{
	std::string mText;
	cFont2 *mFont;
	std::unique_ptr<cSimpleMultiSprite> mSprite;
public:
	cSimpleStringListBoxItem(std::string Text, cFont2 *Font);
	virtual void SetPosition(cPoint Position) override;
	virtual void Show(cPixieWindow *Window, const cRect &ValidRect, int ZOrder) override;
	virtual void Hide() override;
};

class cSimpleStringListBoxItemHandler: public cFixedHeightListBoxItemHandler
{
	std::vector<std::string> mItems;
	int mItemHeight=1;
	cFont2 *mFont;
	void Init();
public:
	cSimpleStringListBoxItemHandler();
	template<class T> cSimpleStringListBoxItemHandler(T &&Items): mItems(std::forward<T>(Items)) { Init(); }
	void SetItemHeight(int Height);
	virtual size_t GetNumberOfItems() const override;
	virtual int GetItemSize() const override;
	virtual std::shared_ptr<cListBoxItem> GetItem(size_t Index) override;

	void InsertString(size_t Position, const std::string &Text);
	void AddString(const std::string &Text);

    void clear();
    void push_back(const std::string& text) { AddString(text); }
    void insert(size_t position, const std::string& text) { InsertString(position, text); }
};

