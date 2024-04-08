#pragma once

struct cPixieInitData
{
	std::shared_ptr<cConfig> mConfig;

	cRect mPlacement;
	cPixieWindow *mParentWindow=nullptr;
	int mZOrder=100;
	int mVisualizerBaseZ=100;
	cResourceLocation mBindTo;
	std::string mVisualizer = "standard"s;
	unsigned int mFlags=0;

	void BindToFromConfig(const cConfig &Config);
	cPixieInitData();
	void setConfig(std::shared_ptr<cConfig> config); // also reads config
};

template<class BaseClass> struct tPixieSimpleInitData: public BaseClass::cInitData
{
	tPixieSimpleInitData()=default;
	void setConfig(std::shared_ptr<cConfig> config)
    {
        BaseClass::cInitData::setConfig(config);
    }
};
