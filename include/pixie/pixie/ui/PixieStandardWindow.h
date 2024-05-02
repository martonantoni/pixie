#pragma once

class cStandardWindow: public cPixieWindow
{
public:
	typedef tVisualizerBase<cStandardWindow> cVisualizer;
	constexpr static int Flag_IsMoveable = 0x1000;  // this is within its parentwindow
	cStandardWindow() {}
	using cInitData=tPixieSimpleInitData<cPixieWindow>;
	void Init(const cInitData &InitData);
protected:
	HAS_VISUALIZER();
private:
	class cDragHandler : public cMouseTarget
	{
		cPoint mLastDragPos;
		bool mIsDragging = false;
		virtual void OnMouseMove(cPoint ScreenCoords, bool IsInside) override;
		virtual void OnLeftButtonDown(cPoint ScreenCoords, bool IsInside) override;
		virtual void OnLeftButtonUp(cPoint ScreenCoords, bool IsInside) override;
	public:
		cDragHandler(cPixieWindow& Window);
	};
	std::unique_ptr<cDragHandler> mDragHandler;
};
