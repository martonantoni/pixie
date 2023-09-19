#pragma once

class cSpriteRenderer;

class cPixieDesktop: public cPixieWindow
{
	std::unique_ptr<cSpriteRenderer> mRenderer;
	cPixieWindow mTopLayer;
	cRegisteredIDList mMouseEventListeners;
	tRegisteredObjects<cMouseTarget *> mMouseTrackers;
	cMouseTarget *mLastMoveTarget=nullptr;
	bool mMouseEventShiftState = false;
	void handleMouseEvent(const cEvent &Event, void (cMouseTarget::*MouseEventHandlerFunction)(cPoint Point, bool IsInside));
	void handleMouseWheel(const cEvent& event);
public:
	typedef tPixieSimpleInitData<cPixieWindow> cInitData;
	void Init(const cInitData &InitData);
	cPixieDesktop();
	~cPixieDesktop()=default;
	cPixieWindow &GetTopLayer() { return mTopLayer; }
	cRegisteredID RegisterMouseTracker(cMouseTarget *MouseTarget);
	void MouseTargetRemoved(cMouseTarget *MouseTarget);
	bool IsReachable(const cPixieWindow *Window) const;
	bool mouseEventShiftState() const { return mMouseEventShiftState; } // hack until the virt. functions in cMouseTarget are improved
};

extern cPixieDesktop thePixieDesktop;