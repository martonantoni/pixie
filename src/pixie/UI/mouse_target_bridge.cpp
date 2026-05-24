#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

void cMouseTargetBridge::init(const cInitData &initData)
{
	cMouseTarget::Init(initData);
    mCallbacks = initData.mCallbacks;
}

void cMouseTargetBridge::OnMouseMove(cPoint screenCoords, bool isInside)
{
	if(isInside==mWasLastMoveInside)
		return;
	mWasLastMoveInside=isInside;
	if(mCallbacks.mOnMouseMove)
		mCallbacks.mOnMouseMove(screenCoords, isInside);
	if (isInside && mCallbacks.mOnMouseEntered)
		mCallbacks.mOnMouseEntered(screenCoords, isInside);

	else if(!isInside && mCallbacks.mOnMouseLeft)
		mCallbacks.mOnMouseLeft(screenCoords, isInside);
}

void cMouseTargetBridge::OnLeftButtonDown(cPoint screenCoords, bool isInside)
{
	if(mCallbacks.mOnLeftButtonDown)
		mCallbacks.mOnLeftButtonDown(screenCoords, isInside);
}

void cMouseTargetBridge::OnLeftButtonUp(cPoint screenCoords, bool isInside)
{
	if(mCallbacks.mOnLeftButtonUp)
		mCallbacks.mOnLeftButtonUp(screenCoords, isInside);
}

void cMouseTargetBridge::OnLeftButtonDoubleClick(cPoint screenCoords, bool isInside)
{
	if(mCallbacks.mOnLeftButtonDoubleClick)
		mCallbacks.mOnLeftButtonDoubleClick(screenCoords, isInside);
}

void cMouseTargetBridge::OnRightButtonDown(cPoint screenCoords, bool isInside)
{
	if(mCallbacks.mOnRightButtonDown)
		mCallbacks.mOnRightButtonDown(screenCoords, isInside);
}

void cMouseTargetBridge::OnRightButtonUp(cPoint screenCoords, bool isInside)
{
	if(mCallbacks.mOnRightButtonUp)
		mCallbacks.mOnRightButtonUp(screenCoords, isInside);
}

std::unique_ptr<cMouseTargetBridge> cMouseTargetBridge::createOverWindow(cPixieWindow& window, const cMouseCallbacks& callbacks)
{
    cMouseTargetBridge::cInitData initData;
    initData.mParentWindow = &window;
    initData.mPlacement = window.GetClientRect();
    initData.mCallbacks = callbacks;
    auto target = std::make_unique<cMouseTargetBridge>();
    target->init(initData);
    return target;
}
