#pragma once

struct cPixieInitData
{
	tIntrusivePtr<cConfig> mConfig;

	cRect mPlacement;
	cPixieWindow *mParentWindow=nullptr;
	int mZOrder=100;
	int mVisualizerBaseZ=100;
	cResourceLocation mBindTo;
	std::string mVisualizer = "standard"s;
	unsigned int mFlags=0;

	void BindToFromConfig(const cConfig &Config);
	cPixieInitData();
	void setConfig(tIntrusivePtr<cConfig> config); // also reads config
};

template<class BaseClass> struct tPixieSimpleInitData: public BaseClass::cInitData
{
	tPixieSimpleInitData()=default;
	void setConfig(tIntrusivePtr<cConfig> config)
    {
        BaseClass::cInitData::setConfig(config);
    }
};
