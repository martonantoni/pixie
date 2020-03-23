#pragma once

class cPixieWindow::cSpriteIterator
{
	typedef std::pair<cWindows::const_reverse_iterator, cWindows::const_reverse_iterator> cSubWindowIterator; // it, end
	typedef std::vector<cSubWindowIterator> cSubWindowIterators;
	cSubWindowIterators mSubWindowIterators;
	cSprites::const_iterator SpritesIt,SpritesItEnd;
	void PushWindow(const cPixieWindow &Window);
	void StepWindowsUntilNonEmptySpriteList();
public:
	cSpriteIterator(const cPixieWindow &BaseWindow);
	cSpriteBase &operator *() const;
	cSpriteBase *operator->() const { return &(**this); }
	cSpriteIterator &operator++();
	bool IsAtEnd() const;
};
