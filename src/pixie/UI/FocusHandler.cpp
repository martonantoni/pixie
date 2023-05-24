#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cFocusHandler theFocusHandler;

cRegisteredID cFocusHandler::AcquireFocus(cDialogItem *Item)
{
	if(Item==mFocusedItem)
		return cRegisteredID(this, mIDCounter);
	auto *OldFocusedItem=mFocusedItem;
	++mIDCounter;
	mFocusedItem=Item;
	if(OldFocusedItem)
		OldFocusedItem->LostFocus();
	return cRegisteredID(this, mIDCounter);
}

void cFocusHandler::Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType)
{
	ASSERT(CallbackType==eCallbackType::Wait);
	if(mIDCounter!=RegisteredID.GetID())
		return;
	mFocusedItem=nullptr;
}
