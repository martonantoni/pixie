#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cVariableHeightStringListBoxItem::cVariableHeightStringListBoxItem(std::shared_ptr<cTextRenderer> TextRenderer, const std::string &Text)
	: mText(Text)
	, mTextRenderer(std::move(TextRenderer))
{
}

void cVariableHeightStringListBoxItem::Render(int MaxWidth)
{
	mTextRenderer->setMaxWidth(MaxWidth-6);
	cTextRenderer::cRenderInfo RenderInfo;
	mSprite=std::make_unique<cSimpleMultiSprite>(mTextRenderer->render(mText, &RenderInfo), cSimpleMultiSprite::eBasePosition::Zero);
	mHeight=RenderInfo.mNextLineY+6;
}

void cVariableHeightStringListBoxItem::SetPosition(cPoint Position)
{
	mSprite->SetPosition(Position+cPoint { 3,3 });
}

void cVariableHeightStringListBoxItem::Show(cPixieWindow *Window, const cRect &ValidRect, int ZOrder)
{
	mSprite->SetWindow(Window);
	mSprite->SetZOrder(ZOrder);
	mSprite->SetValidRect(ValidRect);
	mSprite->Show();
}

void cVariableHeightStringListBoxItem::Hide()
{
	mSprite->Hide();
}

/// --------------------- ItemHandler -----------------------

cVariableHeightStringListBoxItemHandler::cVariableHeightStringListBoxItemHandler()
{
	Init();
}

cVariableHeightStringListBoxItemHandler::cVariableHeightStringListBoxItemHandler(const std::vector<std::string> &Items)
{
	Init();
	mItems.reserve(Items.size());
	for(auto &Text: Items)
	{
		mItems.emplace_back(std::make_shared<cVariableHeightStringListBoxItem>(mTextRenderer, Text));
	}
}

void cVariableHeightStringListBoxItemHandler::Init()
{
	mTextRenderer=std::make_shared<cTextRenderer>(theFontManager.GetFont("listbox_item"));
}

void cVariableHeightStringListBoxItemHandler::setTabStops(std::vector<int> &&TabStops)
{
	mTextRenderer->setTabStops(std::move(TabStops));
}

size_t cVariableHeightStringListBoxItemHandler::GetNumberOfItems() const
{
	return mItems.size();
}

std::shared_ptr<cListBoxItem> cVariableHeightStringListBoxItemHandler::GetItem(size_t Index)
{
	if(ASSERTFALSE(Index>=mItems.size()))
		return nullptr;
	if(!mIsItemPropsValid)
		const_cast<cVariableHeightStringListBoxItemHandler *>(this)->UpdateItems();
	return mItems[Index];
}

void cVariableHeightStringListBoxItemHandler::InsertString(size_t Position, const std::string &Text)
{
	if(ASSERTFALSE(Position>mItems.size()))
		Position=mItems.size();
	mItems.insert(mItems.begin()+Position, std::make_shared<cVariableHeightStringListBoxItem>(mTextRenderer, Text));
	mIsItemPropsValid=false;
	mListChangeDispatcher->PostEvent();
}

void cVariableHeightStringListBoxItemHandler::AddString(const std::string &Text)
{
	mItems.emplace_back(std::make_shared<cVariableHeightStringListBoxItem>(mTextRenderer, Text));
	mIsItemPropsValid=false;
	mListChangeDispatcher->PostEvent();
}

void cVariableHeightStringListBoxItemHandler::UpdateItems()
{
	if(ASSERTFALSE(!mListBox))
		return;
	int MaxWidth=mListBox->GetItemsRect().Width();
	int y=0;
	for(auto &Item: mItems)
	{
		Item->SetGlobalPosition(y);
		if(!Item->IsRendered())
			Item->Render(MaxWidth);
		y+=Item->GetHeight();
	}
	mIsItemPropsValid=true;
}

size_t cVariableHeightStringListBoxItemHandler::GetItemIndexAt(int Position)
{
	if(!mIsItemPropsValid)
		const_cast<cVariableHeightStringListBoxItemHandler *>(this)->UpdateItems();
	auto i=std::lower_bound(ALL(mItems), Position, [](auto &Item, int Position) { return Item->GetGlobalPosition()+Item->GetHeight()<Position; });
	if(i==mItems.end())
		return cListBox::npos;
	return i-mItems.begin();
}

int cVariableHeightStringListBoxItemHandler::GetItemSize(size_t ItemIndex) const
{
	if(!mIsItemPropsValid)
		const_cast<cVariableHeightStringListBoxItemHandler *>(this)->UpdateItems();
	if(ASSERTFALSE(ItemIndex>=mItems.size()))
		return 0;
	return mItems[ItemIndex]->GetHeight();
}

int cVariableHeightStringListBoxItemHandler::GetItemPosition(size_t ItemIndex) const
{
	if(!mIsItemPropsValid)
		const_cast<cVariableHeightStringListBoxItemHandler *>(this)->UpdateItems();
	if(ASSERTFALSE(ItemIndex>=mItems.size()))
		return 0;
	return mItems[ItemIndex]->GetGlobalPosition();
}
