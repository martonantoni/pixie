#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"


void cPixieInitData::setConfig(tIntrusivePtr<cConfig> config)
{
	mConfig = std::move(config);
	mVisualizer = mConfig->GetString("visualizer", mVisualizer);
	mPlacement.FromConfig(*mConfig, cRect::eIsOptional::Yes);
	mZOrder = mConfig->GetInt("z", mZOrder);
	mVisualizerBaseZ = mConfig->GetInt("visualizer_base_z", mVisualizerBaseZ);
	BindToFromConfig(*mConfig);
}

cPixieInitData::cPixieInitData()
{
}

void cPixieInitData::BindToFromConfig(const cConfig &Config)
{
	auto BindToConfig= Config.GetString("bind_to", std::string());
	if(!BindToConfig.empty())
		mBindTo.FromString(BindToConfig);
}
