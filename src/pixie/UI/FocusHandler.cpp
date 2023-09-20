#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cFocusHandler theFocusHandler;

cFocusable::~cFocusable()
{
	theFocusHandler.removeFocusable(this);
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
