#pragma once

struct cPixieInitData
{
	tIntrusivePtr<cConfig2> mConfig;

	cRect mPlacement;
	cPixieWindow *mParentWindow=nullptr;
	int mZOrder=100;
	int mVisualizerBaseZ=100;
	cResourceLocation mBindTo;
	std::string mVisualizer = "standard"s;
	unsigned int mFlags=0;

	void BindToFromConfig(const cConfig2 &Config);
	cPixieInitData();
	void setConfig(tIntrusivePtr<cConfig2> config); // also reads config
};

template<class BaseClass> struct tPixieSimpleInitData: public BaseClass::cInitData
{
	tPixieSimpleInitData()=default;
	void setConfig(tIntrusivePtr<cConfig2> config)
    {
        BaseClass::cInitData::setConfig(config);
    }
};
