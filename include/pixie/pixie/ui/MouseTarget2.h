#pragma once

class cMouseTarget: public cPixieObject
{
public:
	enum eEventType: size_t
	{
		Event_PlacementChanged = PixieEvents::MouseTarget_First,
		Event_ParentWindowChanged
	};
	enum eFlags
	{
		Flag_IgnoreBoundingBoxForIsInside = 1,
	};
private:
	static const cEventDispatchers::cDispatcherRangeInfo mDispatcherRangeInfo;
protected:
	cEventDispatchers mEventDispatchers;
public:
	tIntrusivePtr<cEventDispatcher> GetDispatcher(size_t EventIndex) const { return mEventDispatchers[EventIndex]; }
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	using cInitData = cPixieInitData;
private:
	cPixieWindow* mWindow = nullptr;
	cRect mWindowRelativeBoundingBox;
	bool mIgnoreBoundingBoxForIsInside = false;
	cRect mValidRect;
	eClippingMode mClippingMode = eClippingMode::None;
	int mZOrder = 100;
	cRegisteredID mMouseTrackingID;
	static unsigned int mMouseTargetIDCounter;
	unsigned int mMouseTargetID = ++mMouseTargetIDCounter;
protected:
	void StartMouseTracking();
	void StopMouseTracking();
	bool IsMouseTrackingActive() const;
	virtual bool IsInside_Overridable(cPoint WindowRelativePoint) const { return true; } // default: if it is inside bounding box => it is inside
public:
	virtual ~cMouseTarget();
	void Init(const cInitData &InitData);

	bool IsInside(cPoint WindowRelativePoint) const;

	virtual bool GetProperty(unsigned int PropertyFlags, OUT cPropertyValues &Value) const override;
	virtual bool SetProperty(unsigned int PropertyFlags, const cPropertyValues &Value) override;
	virtual bool SetStringProperty(unsigned int PropertyFlags, const std::string &Value) override;

	const cPixieWindow *GetWindow() const { return mWindow; }
	cPixieWindow *GetWindow() { return mWindow; }
	void SetWindow(cPixieWindow *Window);
	cRect GetPlacement() const { return mWindowRelativeBoundingBox; }
	cRect GetScreenPlacement() const;
	void SetPlacement(const cRect &Rect);
	void SetPosition(cPoint Position);
	int GetZOrder() const { return mZOrder; }
	void SetValidRect(const cRect &ValidRect);
	cRect GetValidRect() const { return mValidRect; }
	bool GetUseClipping() const { return mClippingMode != eClippingMode::None; }
	void setClippingMode(eClippingMode ClippingMode);
	cPoint ScreenCoordsToWindowRelativeCoords(cPoint ScreenCoords) const;
	unsigned int GetMouseTargetID() const { return mMouseTargetID; }

	virtual void OnMouseMove(cPoint ScreenCoords, bool IsInside) {}
	virtual void OnLeftButtonDown(cPoint ScreenCoords, bool IsInside) {}
	virtual void OnLeftButtonUp(cPoint ScreenCoords, bool IsInside) {}
	virtual void OnLeftButtonDoubleClick(cPoint ScreenCoords, bool IsInside) {}
	virtual void OnRightButtonDown(cPoint ScreenCoords, bool IsInside) {}
	virtual void OnRightButtonUp(cPoint ScreenCoords, bool IsInside) {}
	virtual void OnMouseWheel(cPoint ScreenCoords, double delta, bool IsInside) {}
};
