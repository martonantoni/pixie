#pragma once

class cTextField: public cDialogItem
{
public:
	struct cInitData: public tPixieSimpleInitData<cMouseTarget>
	{
		std::string mTitle;
		std::string mTextStyle { "text_field" };
		cInitData()=default;
		void setConfig(tIntrusivePtr<cConfig> Config);
	};
	class cVisualizer: public tVisualizerBase<cTextField>
	{
	public:
		virtual void StateChanged()=0;
	};
protected:
	HAS_VISUALIZER();
	virtual void DialogItemStateChanged() override;
public:
	virtual ~cTextField()=default;
	void Init(const cInitData &InitData);
};