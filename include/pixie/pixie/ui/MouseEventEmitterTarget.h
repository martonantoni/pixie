#pragma once

class cMouseEventEmitterTarget: public cMouseTarget
{
public:
	enum eEventType: size_t
	{
		Event_MoveInside = PixieEvents::EventEmitterTarget_First,
		Event_MoveOutside,
		Event_LeftButtonDown, Event_LeftButtonUp, Event_RightButtonDown, Event_RightButtonUp, Event_LeftButtonDoubleClick
	};
	static const cEventDispatchers::cDispatcherRangeInfo mDispatcherRangeInfo;
private:
	bool mWasLastMoveInside=false;
protected:
	virtual void OnMouseMove(cPoint ScreenCoords, bool IsInside) override;
	virtual void OnLeftButtonDown(cPoint ScreenCoords, bool IsInside) override;
	virtual void OnLeftButtonUp(cPoint ScreenCoords, bool IsInside) override;
	virtual void OnLeftButtonDoubleClick(cPoint ScreenCoords, bool IsInside) override;
	virtual void OnRightButtonDown(cPoint ScreenCoords, bool IsInside) override;
	virtual void OnRightButtonUp(cPoint ScreenCoords, bool IsInside) override;
public:
	using cInitData = cMouseTarget::cInitData;
	void Init(const cInitData &InitData);
    static std::unique_ptr<cMouseEventEmitterTarget> CreateOverSprite(const cSpriteBase& sprite);
};

