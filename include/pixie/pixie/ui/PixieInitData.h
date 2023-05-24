#pragma once

struct cPixieInitData
{
	const tIntrusivePtr<cConfig> mConfig;            // constructor sets it
	cRect mPlacement;
	cPixieWindow *mParentWindow=nullptr;
	int mZOrder=100;
	int mVisualizerBaseZ=100;
	cResourceLocation mBindTo;
	std::string mVisualizer;
	unsigned int mFlags=0;

	void BindToFromConfig(const cConfig &Config);
	cPixieInitData();
	cPixieInitData(const tIntrusivePtr<cConfig> &Config);
};

template<class BaseClass> struct tPixieSimpleInitData: public BaseClass::cInitData
{
	tPixieSimpleInitData()=default;
	tPixieSimpleInitData(const tIntrusivePtr<cConfig> &Config): BaseClass::cInitData(Config) {}
};
