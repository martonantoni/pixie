#pragma once

class cPixieWindowVisualizer;

class cPixieWindow
{
	cRect mPlacement; // relative to parent window
	cSpriteRenderer mRenderer;
	cSprite *mWindowSprite;
	tIntrusivePtr<cTexture> mWindowTexture;
	DWORD mAlpha;
	bool mIsTrackingMouse;
	bool mIsVisible;
	int mZIndex;
protected:
	struct cWindowEvent;
	cPixieWindow *mParentWindow;
	typedef std::list<cPixieWindow *> cPixieWindows;
	cPixieWindows mChildWindows;
	BOOL mIsValid;
	void InsertWindowToChildList(cPixieWindow *Window);
	void ChildZOrderChanged(cPixieWindow *Window);

	void StartMouseTracking();
	void StopMouseTracking();

	virtual void InitVisualizer(const cConfig2 &Config)=0;
	virtual void InitSelf(const cConfig2 &Config)=0;
	virtual void Redraw() {}
public:
	cPixieWindow();
	void Init(const cConfig2 &Config);
	void Init(const cRect &Placement,const cConfig2 &Config);
	virtual ~cPixieWindow();
	void RedrawIfNeeded();
	void Invalidate();
	cSpriteRenderer *GetSpriteRenderer() { return &mRenderer; }
	void AddWindow(OWNERSHIP cPixieWindow *Window);
	void RemoveWindow(cPixieWindow *Window); // takes back the OWNERSHIP
	void Show();
	void Hide();
	void SetAlpha(DWORD Alpha); // 0 - 255
	const cRect &GetPlacement() const { return mPlacement; }
	void SetPlacement(const cRect &Placement);
	cPoint ScreenCoordinatesToWindowCoordinates(const cPoint &Point) const;
	cPoint WindowCoordinatesToScreenCoordinates(const cPoint &Point) const;

	cPixieWindow *GetWindowAt(INOUT int &x,INOUT int &y);
	cPixieWindow *GetWindowAt(const cPoint &Coordinates) ;

	enum eHitTestResult { Hit_Inside, Hit_Outsite };
	virtual eHitTestResult HitTest(const cPoint &WindowRelativeCoordinates) const;
	virtual void OnMouseMove(const cPoint &WindowRelativeCoordinates) {} 
	virtual void OnLeftButtonDown(const cPoint &WindowRelativeCoordinates) {}
	virtual void OnLeftButtonUp(const cPoint &WindowRelativeCoordinates) {}
	virtual void OnRightButtonDown(const cPoint &WindowRelativeCoordinates) {}
	virtual void OnRightButtonUp(const cPoint &WindowRelativeCoordinates) {}
};

