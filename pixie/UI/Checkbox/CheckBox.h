#pragma once

class cCheckBox: public cDialogItem
{
	bool mIsChecked;
public:
	typedef tPixieSimpleInitData<cMouseTarget> cInitData;
	class cVisualizer: public tVisualizerBase<cCheckBox>
	{
	public:
		virtual void StateChanged()=0;
	};
protected:
	HAS_VISUALIZER();
	virtual void OnLeftButtonDown(cPoint ScreenCoords, bool IsInside) override;
	virtual void OnLeftButtonUp(cPoint ScreenCoords, bool IsInside) override;
	virtual void Selected();
	virtual void DialogItemStateChanged() override;
public:
	cCheckBox();
	void Init(const cInitData &InitData);
	bool IsChecked() const { return mIsChecked; }
	virtual void SetCheckState(bool IsChecked);
};