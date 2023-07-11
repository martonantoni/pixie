#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

cPixieInitData::cPixieInitData(const cConfig& config)
	: mConfig(config)
{
	mVisualizer=mConfig.GetString("visualizer", "standard"s);
	mPlacement.FromConfig(mConfig, cRect::eIsOptional::Yes);
	mZOrder=mConfig.GetInt("z", mZOrder);
	mVisualizerBaseZ=mConfig.GetInt("visualizer_base_z", mVisualizerBaseZ);
	BindToFromConfig(mConfig);
}

cPixieInitData::cPixieInitData()
	: mConfig(*cEmptyConfig::theEmptyConfig())
{
}

void cPixieInitData::BindToFromConfig(const cConfig &Config)
{
	auto BindToConfig=mConfig.GetString("bind_to", std::string());
	if(!BindToConfig.empty())
		mBindTo.FromString(BindToConfig);
}
