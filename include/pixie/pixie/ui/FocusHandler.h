#pragma once

class cFocusable
{
public:
	virtual ~cFocusable();
	virtual void onAcquiredFocus() {}
	virtual void onLostFocus() {}
    bool isInFocus() const;
	void acquireFocus();
	void dropFocus();
};

class cFocusHandler
{
	cFocusable* mFocusedItem=nullptr;
public:
	void addFocusable(cFocusable* focusable);
	void removeFocusable(cFocusable* focusable);
	void setFocusedItem(cFocusable* focusable);
	cFocusable* focusedItem() const { return mFocusedItem; }
};

extern cFocusHandler theFocusHandler;