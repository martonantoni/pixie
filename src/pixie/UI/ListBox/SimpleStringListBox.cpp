#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cSimpleStringListBoxItem::cSimpleStringListBoxItem(std::string Text, cFont2 *Font)
	: mText(std::move(Text))
	, mFont(Font)
{
}

void cSimpleStringListBoxItem::SetPosition(cPoint Position)
{
	mSprite->SetPosition(Position);
}

void cSimpleStringListBoxItem::Show(cPixieWindow *Window, const cRect &ValidRect, int ZOrder)
{
	cTextRenderer TextRenderer(mFont);
	mSprite=std::make_unique<cSimpleMultiSprite>(TextRenderer.RenderText(mText), cSimpleMultiSprite::eBasePosition::Zero);
	mSprite->SetWindow(Window);
	mSprite->SetZOrder(ZOrder);
	mSprite->SetValidRect(ValidRect);
	mSprite->Show();
}

void cSimpleStringListBoxItem::Hide()
{
	mSprite->Hide();
}

/// --------------------- ItemHandler -----------------------

cSimpleStringListBoxItemHandler::cSimpleStringListBoxItemHandler()
{
	Init();
}

void cSimpleStringListBoxItemHandler::Init()
{
	mFont=theFontManager.GetFont("listbox_item");
	mItemHeight=mFont->GetHeight();
}

void cSimpleStringListBoxItemHandler::SetItemHeight(int Height)
{
	mItemHeight=Height;
}

size_t cSimpleStringListBoxItemHandler::GetNumberOfItems() const
{
	return mItems.size();
}

int cSimpleStringListBoxItemHandler::GetItemSize() const
{
	return mItemHeight;
}

std::shared_ptr<cListBoxItem> cSimpleStringListBoxItemHandler::GetItem(size_t Index)
{
	if(ASSERTFALSE(Index>=mItems.size()))
		return nullptr;
	return std::make_shared<cSimpleStringListBoxItem>(mItems[Index], mFont);
}

void cSimpleStringListBoxItemHandler::InsertString(size_t Position, const std::string &Text)
{
	if(ASSERTFALSE(Position>mItems.size()))
		Position=mItems.size();
	mItems.insert(mItems.begin()+Position, Text);
	mListChangeDispatcher->PostEvent();
}

void cSimpleStringListBoxItemHandler::AddString(const std::string &Text)
{
	mItems.emplace_back(Text);
	mListChangeDispatcher->PostEvent();
}

void cSimpleStringListBoxItemHandler::clear()
{
    mItems.clear();
    mListChangeDispatcher->PostEvent();
}

