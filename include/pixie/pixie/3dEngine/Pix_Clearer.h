#pragma once

class cDeviceClearer abstract
{
public:
	virtual ~cDeviceClearer() {}
	virtual void ClearDevice(IDirect3DDevice9 *Device) abstract;
};

class cBasicDeviceClearer: public cDeviceClearer
{
	D3DCOLOR Color;
public:
	void Init(const cConfig &Config);
	void ClearDevice(IDirect3DDevice9 *Device) override;
};
