#pragma once

class cSpriteRenderer;
class cGlowPostProcessor;

enum class eDisplayMode { Color, Glow, Emission };

class cPixieDesktop: public cRootWindow
{
    std::unique_ptr<cGlowPostProcessor> mGlowPostProcessor;
    eDisplayMode mDisplayMode = eDisplayMode::Glow;
	cPixieWindow mTopLayer;
	cRegisteredIDList mMouseEventListeners;
	tRegisteredObjects<cMouseTarget *> mMouseTrackers;
	cMouseTarget *mLastMoveTarget=nullptr;
	std::vector<cPixieWindow *> mOwnerlessWindows;
	bool mMouseEventShiftState = false;
	bool mMouseEventCtrlState = false;
	bool mMouseEventAltState = false;
	void handleMouseEvent(const cEvent &Event, void (cMouseTarget::*MouseEventHandlerFunction)(cPoint Point, bool IsInside));
	void handleMouseWheel(const cEvent& event);
public:
	typedef tPixieSimpleInitData<cPixieWindow> cInitData;
	void Init(const cInitData &InitData);
    cPixieDesktop() = default;
    void render() override;
    void setDisplayMode(eDisplayMode mode) { mDisplayMode = mode; }
    eDisplayMode displayMode() const { return mDisplayMode; }
	virtual ~cPixieDesktop();
	cPixieWindow &GetTopLayer() { return mTopLayer; }
	cRegisteredID RegisterMouseTracker(cMouseTarget *MouseTarget);
	void MouseTargetRemoved(cMouseTarget *MouseTarget);
	bool IsReachable(const cPixieWindow *Window) const;
	// hack until the virt. functions in cMouseTarget are improved:
	bool mouseEventShiftState() const { return mMouseEventShiftState; } 
	bool mouseEventCtrlState() const { return mMouseEventCtrlState; }
	bool mouseEventAltState() const { return mMouseEventAltState; }

	void addOwnerlessWindow(cPixieWindow* window) { mOwnerlessWindows.push_back(window); }
	void removeOwnerlessWindow(cPixieWindow* window) { std::erase(mOwnerlessWindows, window); }
	auto ownerlessWindows() const { return mOwnerlessWindows; }
};

extern cPixieDesktop thePixieDesktop;