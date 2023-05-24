#include "StdAfx.h"

cPixieInitData::cPixieInitData(const tIntrusivePtr<cConfig> &Config)
	: mConfig(Config)
{
	if(ASSERTFALSE(!mConfig))
		return; // not supported at all, crash imminent
	mVisualizer=mConfig->GetString("visualizer", "standard"s);
	mPlacement.FromConfig(*mConfig, cRect::eIsOptional::Yes);
	mZOrder=mConfig->GetInt("z", mZOrder);
	mVisualizerBaseZ=mConfig->GetInt("visualizer_base_z", mVisualizerBaseZ);
	BindToFromConfig(*Config);
}

cPixieInitData::cPixieInitData()
	: mConfig(cEmptyConfig::GetEmptyConfig())
{
}

void cPixieInitData::BindToFromConfig(const cConfig &Config)
{
	auto BindToConfig=mConfig->GetString("bind_to", std::string());
	if(!BindToConfig.empty())
		mBindTo.FromString(BindToConfig);
}
