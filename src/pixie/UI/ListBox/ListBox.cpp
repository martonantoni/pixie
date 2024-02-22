#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
INIT_VISUALIZER_IMP(cListBox);

const cEventDispatchers::cDispatcherRangeInfo cListBox::mDispatcherRangeInfo=
{
	PixieEvents::ListBox_First, { "selection_changed", "double_clicked" }
};

cDampenedSpeedCalculator::cDampenedSpeedCalculator(const cPositionReceiverFunction &PositionDeltaReceiver)
	: mPositionDeltaReceiver(PositionDeltaReceiver)
{
	ASSERT(mPositionDeltaReceiver);
}

void cDampenedSpeedCalculator::SetSpeed(double Speed)
{
	mLastTime=gFrameTime;
	mSpeed=Speed;
	if(std::abs(mSpeed)<EPSILON)
	{
		mSpeed=0.0;
		mLogicID.Unregister();
		return;
	}
	mLogicID=theLogicServer.AddLogic([this]()
	{
		if(gFrameTime==mLastTime)
			return;
		auto DeltaTimeMS=gFrameTime-mLastTime;
		DeltaTimeMS=std::min(100u, DeltaTimeMS);
		mLastTime=gFrameTime;
		double DeltaTimeSeconds=DeltaTimeMS/1000.0;
		auto PositionDelta=mSpeed*DeltaTimeSeconds;
		mSpeed=0.8*mSpeed;
		mPositionDeltaReceiver(PositionDelta);
		if(std::abs(mSpeed)<EPSILON)
		{
			mLogicID.Unregister();
			return;
		}
	},100);
}

void cDampenedSpeedCalculator::AddToSpeed(double Speed)
{
	if(!mLogicID.IsValid())
	{
		SetSpeed(Speed);
		return;
	}
	if(mSpeed<0.0&&Speed<0.0)
	{
		SetSpeed(mSpeed+Speed);
		return;
	}
	if(mSpeed>0.0&&Speed>0.0)
	{
		SetSpeed(mSpeed+Speed);
		return;
	}
	SetSpeed(Speed);
}

void cDragSpeedCalculator::DragStart(int Position)
{
	mIsDragging=true;
	mPrevPosition=Position;
	mPrevTime=gFrameTime-50;
	mSpeed=0.0;
}

double cDragSpeedCalculator::DragEnd()
{
	if(gFrameTime-mPrevTime>120)
		mSpeed=0.0;
	mIsDragging=false;
	return mSpeed;
}

void cDragSpeedCalculator::DraggedToPosition(int Position)
{
	ASSERT(mIsDragging);
	if(gFrameTime-mPrevTime>50)
	{
		auto OldPosition=mPrevPosition;
		auto OldTime=mPrevTime;
		mPrevPosition=Position;
		mPrevTime=gFrameTime;
		auto DeltaTimeMS=mPrevTime-OldTime;
		double DeltaTimeSec=DeltaTimeMS/1000.0;
		auto DeltaPosition=mPrevPosition-OldPosition;
		mSpeed=DeltaPosition/DeltaTimeSec;
		MainLog->Log("Speed: %.3f", mSpeed);
	}
}

cListBox::cListBox()
	:mDampenedMoveController([this](int PositionDelta) { SetGlobalPosition(mTopLeftPosition.y+PositionDelta); })
{
}

cListBox::~cListBox()
{
	mItemHandler->SetListBox(nullptr);
}

void cListBox::Init(const cInitData &InitData)
{
	cDialogItem::Init(InitData);
	mListDirection=InitData.mListDirection;
	mItemHighlightEnabled=InitData.mItemHighlightEnabled;
	mItemSelectEnabled=InitData.mItemSelectEnabled;
	mEventDispatchers.AddEvents(mDispatcherRangeInfo);
	mEventDispatchers[Event_SelectionChanged]->SetCoalescing();
	InitVisualizer(InitData, InitData.mVisualizer);
	mVisualizer->SetItemHandler(InitData.mItemHandler);
	mItemHandler=InitData.mItemHandler;
	mItemHandler->SetListBox(this);
	mListChangeListenerID=mItemHandler->GetListChangeDispatcher()->RegisterListener([this](auto &) { ListChanged(); });

// 	mMouseWheelListeningID=cEventDispatcher::GetGlobalDispatcher("pixie.mouse.wheel")->RegisterListener(
// 		[this](auto &Event)
// 	{
//         if (thePixieDesktop.GetMouseTargetAt(cMouseServer::Get().GetMousePosition()).mResult == this)
//         {
//             const int* Delta = tDataHolder<int>::Get().GetData(Event.mEventDataID);
//             mDampenedMoveController.AddToSpeed(*Delta * 4.0);
//         }
// 	});
}

void cListBox::clampGlobalPosition()
{
    if (mTopLeftPosition.x < 0 || mListDirection == eListDirection::Vertical)
        mTopLeftPosition.x = 0;
    if (mTopLeftPosition.y < 0 || mListDirection == eListDirection::Horizontal)
        mTopLeftPosition.y = 0;
    if (mListDirection == eListDirection::Vertical)
    {
        size_t numItems = mItemHandler->GetNumberOfItems();
        int lastItemPos = numItems > 0 ? mItemHandler->GetItemPosition(numItems - 1) : 0;
        if (mTopLeftPosition.y > lastItemPos)
            mTopLeftPosition.y = lastItemPos;
    }
}

void cListBox::OnMouseMove(cPoint ScreenCoords, bool IsInside)
{
	if(!IsEnabled())
		return;
	ScreenCoords-=GetWindow()->GetScreenRect().GetPosition();
	if(IsMouseTrackingActive())
	{
		mTopLeftPosition=mTopLeftPositionWhenGrabbed+mGrabScreenPosition-ScreenCoords;
        clampGlobalPosition();
		mDragSpeedCalculator.DraggedToPosition(mTopLeftPosition.y);
		mVisualizer->StateChanged();
	}
	auto OldHighlightedIndex=mHighlightedItemIndex;
	if(IsInside)
	{
		mHighlightedItemIndex=mVisualizer->GetItemIndexAt(ScreenCoords-mVisualizer->GetItemsRect().GetPosition()+mTopLeftPosition);
	}
	else
	{
		mHighlightedItemIndex=npos;
	}
	if(OldHighlightedIndex!=mHighlightedItemIndex)
		mVisualizer->StateChanged();
}

void cListBox::OnLeftButtonDown(cPoint ScreenCoords, bool IsInside)
{
	if(!IsEnabled())
		return;
	ScreenCoords-=GetWindow()->GetScreenRect().GetPosition();
	if(IsInside)
	{
		mGrabScreenPosition=ScreenCoords;
		mTopLeftPositionWhenGrabbed=mTopLeftPosition;
		StartMouseTracking();
		mDragSpeedCalculator.DragStart(mTopLeftPositionWhenGrabbed.y);
		mDampenedMoveController.SetSpeed(0.0);
		auto SelectedIndex=mVisualizer->GetItemIndexAt(ScreenCoords-mVisualizer->GetItemsRect().GetPosition()+mTopLeftPosition);
		if(SelectedIndex!=npos&&SelectedIndex<mItemHandler->GetNumberOfItems())
		{
			SetSelectionIndex(SelectedIndex);
		}
	}
}

void cListBox::OnLeftButtonDoubleClick(cPoint ScreenCoords, bool IsInside)
{
	if(!IsEnabled())
		return;
	if(GetSelectionIndex()==cListBox::npos)
		return;
	mEventDispatchers.PostEvent(Event_DoubleClicked);
}

void cListBox::OnLeftButtonUp(cPoint ScreenCoords, bool IsInside)
{
	if(!IsEnabled())
		return;
	StopMouseTracking();
	mDampenedMoveController.SetSpeed(mDragSpeedCalculator.DragEnd());
}

void cListBox::DialogItemStateChanged()
{
	if(!IsEnabled())
	{
		StopMouseTracking();
	}
}

void cListBox::SetSelectionIndex(size_t Index)
{
	if(!mItemSelectEnabled)
		return;
	if(mSelectedItemIndex==Index)
		return;
	auto NumberOfItems=mItemHandler->GetNumberOfItems();
	if(Index>=NumberOfItems)
	{
		Index=NumberOfItems-1;
		if(NumberOfItems==0)
			Index=npos;
	}
	mSelectedItemIndex=Index;
	mEventDispatchers.PostEvent(Event_SelectionChanged);
	mVisualizer->StateChanged();
}

size_t cListBox::GetSelectionIndex() const
{
	return mItemSelectEnabled?mSelectedItemIndex:npos;
}

size_t cListBox::GetHighlighedIndex() const
{
	return mItemHighlightEnabled?mHighlightedItemIndex:npos;
}

void cListBox::ListChanged()
{
	if(mSelectedItemIndex>=mItemHandler->GetNumberOfItems())
	{
		mSelectedItemIndex=npos;
		mEventDispatchers.PostEvent(Event_SelectionChanged);
	}
	mVisualizer->InvalidateItems();
	mVisualizer->StateChanged();
}

cRect cListBox::GetItemsRect() const
{
	return mVisualizer->GetItemsRect();
}

bool cListBox::IsItemVisible(size_t Index) const
{
	return mVisualizer->IsItemVisible(Index);
}

void cListBox::MakeItemVisible(size_t Index)
{
	mVisualizer->MakeItemVisible(Index);
}

void cListBox::SetGlobalPosition(int Position)
{
	(mListDirection==eListDirection::Vertical?mTopLeftPosition.y:mTopLeftPosition.x)=std::max(0, Position);
    clampGlobalPosition();
	mVisualizer->StateChanged();
}

/// -----------------------------------------------------------

cListBox::cInitData::cInitData(const cConfig &Config)
	: tPixieSimpleInitData<cMouseTarget>(Config)
{
	mListDirection=Config.GetBool("is_vertical", true)?eListDirection::Vertical:eListDirection::Horizontal;
	mItemHighlightEnabled=Config.GetBool("highlight_enabled", true);
	mItemSelectEnabled=Config.GetBool("select_enabled", true);
}
