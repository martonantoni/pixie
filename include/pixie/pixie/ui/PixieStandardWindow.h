#pragma once

class cStandardWindow: public cPixieWindow
{
public:
	typedef tVisualizerBase<cStandardWindow> cVisualizer;
public:
	cStandardWindow() {}
	using cInitData=tPixieSimpleInitData<cPixieWindow>;
	void Init(const cInitData &InitData);
protected:
	HAS_VISUALIZER();
};
