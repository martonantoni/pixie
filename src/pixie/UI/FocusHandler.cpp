#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cFocusHandler theFocusHandler;

cFocusable::~cFocusable()
{
	theFocusHandler.removeFocusable(this);
}

bool cFocusable::isInFocus() const
{
	return theFocusHandler.focusedItem() == this;
}

void cFocusable::acquireFocus()
{
	theFocusHandler.acquireFocus(this);
}

void cFocusHandler::addFocusable(cFocusable* focusable)
{
}

void cFocusHandler::removeFocusable(cFocusable* focusable)
{
	if (mFocusedItem == focusable)
		mFocusedItem = nullptr;
}

void cFocusHandler::acquireFocus(cFocusable* focusable)
{
	if (mFocusedItem == focusable)
		return;
	if (mFocusedItem)
		mFocusedItem->onLostFocus();
	mFocusedItem = focusable;
	focusable->onAcquiredFocus();
}

