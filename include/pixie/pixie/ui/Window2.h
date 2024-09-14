#pragma once

class cMouseBlocker;

class cPixieWindow: public cPixieObject
{
	friend class cSpriteBase;
	friend class cMouseTarget;
	friend class cSpriteRenderer;
public:
	enum eEventType
	{
		Event_PlacementChanged = PixieEvents::Window_First,
		Event_VisibilityChanged,
		Event_ParentWindowChanged,
	};
    // init flags:
    constexpr static int Flag_ClickThrough = 1;
    constexpr static int Flag_IsModal = 2;  // this is within its parentwindow
	constexpr static int Flag_ClipSpritesToValidRect = 4;
	constexpr static int Flag_ClipSpritesToScreenRect = 4;
private:
	static const cEventDispatchers::cDispatcherRangeInfo mDispatcherRangeInfo;
	cEventDispatchers mEventDispatchers;
public:
	enum class eClipping
	{
		None,
		ClipToClientRect,
		ClipToParentClientRect,
		ClipToValidRect_ScreeCoords,
		ClipToValidRect_WindowCoords,
		ClipToValidRect_ParentWindowCoords,
	};
	struct cInitData : public cPixieInitData
	{
		eClipping mClippingMode = eClipping::None;
		cRect mValidRect;
	};
private:
	cRect mPlacement; // relative to parent window
	cRect mScreenRect;
	cRect mValidRect; 
	eClipping mSpriteClipping = eClipping::None;
	bool mIsClickThrough = false;
	bool mIsModal = false;
	int mZOrder=100;  // 0: "above all subwindows" sprite layer (virtual window)
					  // 1-99 top level windows
					  // 100+ normal windows

	using cWindows = std::vector<cPixieWindow*>;
	cWindows mSubWindows;
	cPixieWindow* mParentWindow = nullptr;

	typedef std::vector<cSpriteBase *> cSprites;
	cSprites mSprites;
	typedef std::vector<cMouseTarget *> cMouseTargets;
	cMouseTargets mMouseTargets;
	class cMouseTargetIterator;
	std::unique_ptr<cMouseBlocker> mMouseBlocker;
	void UpdateScreenRect(const cRect &ParentScreenRect);
	std::pair<bool, cRect> getSpriteClipping() const; // rect is in screen coordinates
protected:
	int mSpriteBaseZ=0;
public:
	cPixieWindow()=default;
	virtual ~cPixieWindow();
	void Init(const cInitData &InitData);
	tIntrusivePtr<cEventDispatcher> GetDispatcher(eEventType EventType) const;
	// Properties (PixieObject):
	virtual bool GetProperty(unsigned int PropertyFlags, OUT cPropertyValues &Value) const override;
	virtual bool SetProperty(unsigned int PropertyFlags, const cPropertyValues &Value) override;
	virtual bool SetStringProperty(unsigned int PropertyFlags, const std::string &Value) override;
	// Position & State:
	const cRect &GetScreenRect() const { return mScreenRect; }
	const cRect &GetPlacement() const { return mPlacement; } // relative to parent window
	cRect GetClientRect() const { return cRect({ 0,0 }, mPlacement.size()); }
	void setPosition(const cPoint &Position);
	void setSize(const cPoint &Size);
	void SetPlacement(const cRect &Rect);
	void SetValidRect(const cRect &ValidRect);
	int GetZOrder() const { return mZOrder; }
	bool IsInside(cPoint WindowRelativeCoords) const;
	cPoint WindowCoordinatesToScreenCoordinates(cPoint WindowRelativeCoords) const;
	cPoint ScreenCoordinatesToWindowCoordinates(cPoint ScreenCoords) const;
	// Sprite handling:
private:
	void AddSprite(cSpriteBase *Sprite);
	void RemoveSprite(cSpriteBase *Sprite);
public:
	void CheckOwnerlessSprites();
	// SubWindows:
private:
	void AddSubWindow(cPixieWindow *SubWindow);
	void RemoveSubWindow(cPixieWindow *SubWindow);
protected:
	enum class eModalityCheckResult { NotFound, Reached, Blocked };
	eModalityCheckResult CheckModality(const cPixieWindow *WindowToReach) const;
public:
	template<class T> struct tGetAtResult;
	enum eGetWindowRules { GetWindow_ClickRules, GetWindow_VisibleRules };
	tGetAtResult<cPixieWindow> GetWindowAt(cPoint WindowRelativeCoordinates, eGetWindowRules GetWindowRules) const; // recursive, returned window might be sub-sub-sub-...window
	void SetParentWindow(cPixieWindow *ParentWindow);
	cPixieWindow *GetParentWindow() { return mParentWindow; }
// MouseTargets:
private:
	void AddMouseTarget(cMouseTarget *Target);
	void RemoveMouseTarget(cMouseTarget *Target);
public:
	tGetAtResult<cMouseTarget> GetMouseTargetAt(cPoint Point) const; // recursive
// misc. helpers:
	cPixieWindow *GetWindow() { return this; } // needed for visualizer templates
};

template<class T> struct cPixieWindow::tGetAtResult
{
	T *mResult=nullptr;
	cPoint mResultRelativeCoords;
	tGetAtResult(T *Result, cPoint ResultRelativeCoords): mResult(Result), mResultRelativeCoords(ResultRelativeCoords) {}
	tGetAtResult()=default;
};

class cMouseBlocker: public cMouseTarget
{
protected:
	virtual bool IsInside_Overridable(cPoint WindowRelativePoint) const override;
};
