#pragma once

class cFocusable
{
public:
	virtual ~cFocusable();
	virtual void onAcquiredFocus() {}
	virtual void onLostFocus() {}
};

class cFocusHandler
{
	cFocusable* mFocusedItem=nullptr;
public:
	void addFocusable(cFocusable* focusable);
	void removeFocusable(cFocusable* focusable);
	void acquireFocus(cFocusable* focusable);
};

extern cFocusHandler theFocusHandler;