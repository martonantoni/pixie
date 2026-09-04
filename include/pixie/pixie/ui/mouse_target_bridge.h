#pragma once

class cMouseTargetBridge: public cMouseTarget
{
public:
    using cMouseFunc = std::function<void(cPoint screenCoords, bool isInside)>;
    struct cMouseCallbacks
    {
        cMouseFunc mOnMouseMove;
        cMouseFunc mOnMouseEntered;
        cMouseFunc mOnMouseLeft;
        cMouseFunc mOnLeftButtonDown;
        cMouseFunc mOnLeftButtonUp;
        cMouseFunc mOnLeftButtonDoubleClick;
        cMouseFunc mOnRightButtonDown;
        cMouseFunc mOnRightButtonUp;
    };
private:
	bool mWasLastMoveInside=false;
    cMouseCallbacks mCallbacks;
protected:
	virtual void OnMouseMove(cPoint screenCoords, bool isInside) override;
	virtual void OnLeftButtonDown(cPoint screenCoords, bool isInside) override;
	virtual void OnLeftButtonUp(cPoint screenCoords, bool isInside) override;
	virtual void OnLeftButtonDoubleClick(cPoint screenCoords, bool isInside) override;
	virtual void OnRightButtonDown(cPoint screenCoords, bool isInside) override;
	virtual void OnRightButtonUp(cPoint screenCoords, bool isInside) override;
public:
	struct cInitData : public cMouseTarget::cInitData
	{
        cMouseCallbacks mCallbacks;
	};
	void init(const cInitData &initData);
    bool wasLastMoveInside() const { return mWasLastMoveInside; }

    void startTracking() { StartMouseTracking(); }
    void stopTracking() { StopMouseTracking(); }
    bool isTrackingActive() const { return IsMouseTrackingActive(); }

    static std::unique_ptr<cMouseTargetBridge> createOverWindow(cPixieWindow& window, const cMouseCallbacks& callbacks);
    static std::unique_ptr<cMouseTargetBridge> createOverSprite(cSpriteBase& sprite, const cMouseCallbacks& callbacks);
};

