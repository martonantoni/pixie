#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
INIT_VISUALIZER_IMP(cStandardWindow);

void cStandardWindow::Init(const cInitData &InitData)
{
	cPixieWindow::Init(InitData);
	InitVisualizer(InitData, InitData.mVisualizer);

	if (InitData.mFlags & Flag_IsMoveable)
	{
		mDragHandler = std::make_unique<cDragHandler>(*this);
	}
}

cStandardWindow::cDragHandler::cDragHandler(cPixieWindow& window)
{
	cMouseTarget::cInitData MouseTargetInitData;
	MouseTargetInitData.mParentWindow = &window;
	MouseTargetInitData.mPlacement = window.GetClientRect();
	MouseTargetInitData.mZOrder = 1;
	cMouseTarget::Init(MouseTargetInitData);
}

void cStandardWindow::cDragHandler::OnMouseMove(cPoint ScreenCoords, bool IsInside)
{
	if (mIsDragging)
	{
		cPoint Delta = ScreenCoords - mLastDragPos;
        mLastDragPos = ScreenCoords;
        cRect CurrentPlacement = GetWindow()->GetPlacement();
        CurrentPlacement.Move(Delta);
		GetWindow()->SetPlacement(CurrentPlacement);
	}
}
void cStandardWindow::cDragHandler::OnLeftButtonDown(cPoint ScreenCoords, bool IsInside)
{
	mIsDragging = true;
	mLastDragPos = ScreenCoords;
}
void cStandardWindow::cDragHandler::OnLeftButtonUp(cPoint ScreenCoords, bool IsInside)
{
	mIsDragging = false;
}