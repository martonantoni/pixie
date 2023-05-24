#include "StdAfx.h"

#include "Window2_SpriteIterator.h"

cPixieWindow::cSpriteIterator::cSpriteIterator(const cPixieWindow &BaseWindow)
{
	PushWindow(BaseWindow);
	StepWindowsUntilNonEmptySpriteList();
}

void cPixieWindow::cSpriteIterator::PushWindow(const cPixieWindow &Window)
{
	mSubWindowIterators.push_back({ Window.mSubWindows.rbegin(), Window.mSubWindows.rend() });
	SpritesIt=Window.mSprites.begin();
	SpritesItEnd=Window.mSprites.end();
}

cSpriteBase &cPixieWindow::cSpriteIterator::operator *() const
{
	return **SpritesIt;
}

cPixieWindow::cSpriteIterator &cPixieWindow::cSpriteIterator::operator++()
{
	if(++SpritesIt!=SpritesItEnd)
	{
		return *this;
	}
	StepWindowsUntilNonEmptySpriteList();
	return *this;
}

void cPixieWindow::cSpriteIterator::StepWindowsUntilNonEmptySpriteList()
{
	while(SpritesIt==SpritesItEnd&&!mSubWindowIterators.empty())
	{
		auto &SubWindowIt=mSubWindowIterators.back();
		if(SubWindowIt.first!=SubWindowIt.second)
		{
			auto &Window=**SubWindowIt.first;
			++SubWindowIt.first;
			PushWindow(Window); // invalidates SubWindowIt
		}
		else
		{
			mSubWindowIterators.pop_back();
		}
	} 
}

bool cPixieWindow::cSpriteIterator::IsAtEnd() const
{
	return SpritesIt==SpritesItEnd;
}

