#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

namespace
{
    void ArgbToFloat4(uint32_t color, float result[4])
    {
        result[0] = ((color >> 16) & 0xff) / 255.0f;
        result[1] = ((color >> 8) & 0xff) / 255.0f;
        result[2] = (color & 0xff) / 255.0f;
        result[3] = ((color >> 24) & 0xff) / 255.0f;
    }
}

void cBasicDeviceClearer::Init(const cConfig &Config)
{
    Color = 0x282828;
    cDevice::Get()->SetClearer(this);
}

void cBasicDeviceClearer::ClearDevice(ID3D11DeviceContext *DeviceContext)
{
    ID3D11RenderTargetView *renderTarget = nullptr;
    DeviceContext->OMGetRenderTargets(1, &renderTarget, nullptr);
    if (!renderTarget)
        return;

    float color[4];
    ArgbToFloat4(Color, color);
    DeviceContext->ClearRenderTargetView(renderTarget, color);
    renderTarget->Release();
}
