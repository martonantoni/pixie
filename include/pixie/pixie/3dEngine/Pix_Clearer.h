#pragma once

#include <d3d11.h>
#include <cstdint>

class cDeviceClearer abstract
{
public:
    virtual ~cDeviceClearer() {}
    virtual void ClearDevice(ID3D11DeviceContext *DeviceContext) abstract;
};

class cBasicDeviceClearer: public cDeviceClearer
{
    uint32_t Color = 0;
public:
    void Init(const cConfig &Config);
    void ClearDevice(ID3D11DeviceContext *DeviceContext) override;
};
