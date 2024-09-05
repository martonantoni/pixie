#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"


void cPixieInitData::setConfig(std::shared_ptr<cConfig> config)
{
	mConfig = std::move(config);
	mVisualizer = mConfig->get<std::string>("visualizer", mVisualizer);
	mPlacement.fromConfig(*mConfig, cRect::eIsOptional::Yes);
	mZOrder = mConfig->get<int>("z", mZOrder);
	mVisualizerBaseZ = mConfig->get<int>("visualizer_base_z", mVisualizerBaseZ);
	BindToFromConfig(*mConfig);
}

cPixieInitData::cPixieInitData()
{
}

void cPixieInitData::BindToFromConfig(const cConfig& config)
{
	auto BindToConfig= config.get<std::string>("bind_to", std::string());
	if(!BindToConfig.empty())
		mBindTo.fromString(BindToConfig);
}
