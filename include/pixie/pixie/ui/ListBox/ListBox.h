#pragma once

class cListBox;

class cListBoxItem
{
public:
	virtual void SetPosition(cPoint Position)=0;
	virtual void Show(cPixieWindow *Window, const cRect &ValidRect, int ZOrder)=0;
	virtual void Hide()=0;
};

class cListBoxItemHandler
{
protected:
	tIntrusivePtr<cEventDispatcher> mListChangeDispatcher;
	cListBox *mListBox=nullptr;
public:
	cListBoxItemHandler(): mListChangeDispatcher(make_intrusive_ptr<cEventDispatcher>(cEventDispatcher::eCoalescing::Coalescing)) {}
	virtual size_t GetNumberOfItems() const=0;
	virtual size_t GetItemIndexAt(int Position)=0;
	virtual int GetItemSize(size_t ItemIndex) const=0;
	virtual int GetItemPosition(size_t ItemIndex) const=0;
	virtual std::shared_ptr<cListBoxItem> GetItem(size_t Index)=0;
	tIntrusivePtr<cEventDispatcher> GetListChangeDispatcher() const { return mListChangeDispatcher; }
	void SetListBox(cListBox *ListBox) { mListBox=ListBox; }
};

class cFixedHeightListBoxItemHandler: public cListBoxItemHandler
{
protected:
	virtual int GetItemSize() const=0;
public:
	virtual size_t GetItemIndexAt(int Position) override { return Position/GetItemSize(); }
	virtual int GetItemSize(size_t ItemIndex) const override { return GetItemSize(); }
	virtual int GetItemPosition(size_t ItemIndex) const override { return (int)ItemIndex*GetItemSize(); }
};

class cDampenedSpeedCalculator
{
public:
	using cPositionReceiverFunction = std::function<void(int PositionDelta)>;
private:
	cPositionReceiverFunction mPositionDeltaReceiver;
	cRegisteredID mLogicID;

	double mSpeed;
	unsigned int mLastTime; // ms
public:
	cDampenedSpeedCalculator(const cPositionReceiverFunction &PositionDeltaReceiver);
	void SetSpeed(double Speed);
	void AddToSpeed(double Speed);
};

class cDragSpeedCalculator
{
	double mPrevPosition;
	unsigned int mPrevTime;
	double mSpeed;
	bool mIsDragging=false;
public:
	void DragStart(int Position);
	double DragEnd(); // returns speed
	void DraggedToPosition(int Position); 
};

// there's two primary modes for ListBoxItems:
// 1) the ListBoxItemHandler creates and maintains a cListBoxItem for each item. the cListBoxItems receive Show when they need to be displayed and Hide
//    when they are no longer visible. In between they may receive any number of SetPosition calls. 
// 2) the ListBoxItemHandler creates the cListBoxItems on demand (and probably does not even hold a reference to them).

class cListBox: public cDialogItem
{
public:
	enum eEventType: size_t
	{
		Event_SelectionChanged = PixieEvents::ListBox_First,
		Event_DoubleClicked,
	};
	static const size_t npos=~0u;
private:
	static const cEventDispatchers::cDispatcherRangeInfo mDispatcherRangeInfo;
public:
	class cVisualizer;
	enum class eListDirection { Vertical, Horizontal };
	struct cInitData: public tPixieSimpleInitData<cMouseTarget>
	{
		std::shared_ptr<cListBoxItemHandler> mItemHandler;
		eListDirection mListDirection=eListDirection::Vertical;
		bool mItemHighlightEnabled=true;
		bool mItemSelectEnabled=true;
		cInitData()=default;
		cInitData(const cConfig& Config);
	};
protected:
	eListDirection mListDirection=eListDirection::Vertical;
	bool mItemHighlightEnabled=true;
	bool mItemSelectEnabled=true;
	cRegisteredID mMouseWheelListeningID;
	std::shared_ptr<cListBoxItemHandler> mItemHandler;
	cRegisteredID mListChangeListenerID;
	cPoint mTopLeftPosition { 0,0 };
	cPoint mGrabScreenPosition;
	cPoint mTopLeftPositionWhenGrabbed;
	size_t mSelectedItemIndex=npos;
	size_t mHighlightedItemIndex=npos;
	cDragSpeedCalculator mDragSpeedCalculator;
	cDampenedSpeedCalculator mDampenedMoveController;
	HAS_VISUALIZER();
	virtual void OnMouseMove(cPoint ScreenCoords, bool IsInside) override;
	virtual void OnLeftButtonDown(cPoint ScreenCoords, bool IsInside) override;
	virtual void OnLeftButtonUp(cPoint ScreenCoords, bool IsInside) override;
	virtual void OnLeftButtonDoubleClick(cPoint ScreenCoords, bool IsInside) override;
	virtual void DialogItemStateChanged() override;
    void clampGlobalPosition();
public:
	cListBox();
	virtual ~cListBox();

// functions for ListBoxItemHandlers:
	cRect GetItemsRect() const;

// functions for the visualizers:
	cPoint Vis_GetPosition() const { return mTopLeftPosition; }
	eListDirection Vis_GetListDirection() const { return mListDirection; }
	int Vis_GetGlobalPosition() const
	{
		return Vis_GetListDirection()==eListDirection::Vertical?Vis_GetPosition().y:Vis_GetPosition().x;
	}

// functions for the users of ListBox:
	void Init(const cInitData &InitData);
	void SetSelectionIndex(size_t Index);

// common functions:
	size_t GetSelectionIndex() const;
	size_t GetHighlighedIndex() const;
	void ListChanged();
	bool IsItemVisible(size_t Index) const;
	void MakeItemVisible(size_t Index);
	void SetGlobalPosition(int Position);
};

