#include "StdAfx.h"

#include "Pix_Clearer.h"
#include "Pix_Device.h"

void cBasicDeviceClearer::Init(const cConfig &Config)
{
	// ARGB
//	Color=0x40604000;
//	Color=0x00808080;
	Color=0;// 0xff0000;//0x00808080;//0x00202020;
//	Color=0x2f4822;

	cDevice::Get()->SetClearer(this);
}

void cBasicDeviceClearer::ClearDevice(IDirect3DDevice9 *Device)
{
	StopOnError(Device->Clear(0,  //Number of rectangles to clear, we're clearing everything so set it to 0
		NULL, //Pointer to the rectangles to clear, NULL to clear whole display
		D3DCLEAR_TARGET,   //What to clear.  We don't have a Z Buffer or Stencil Buffer
		Color, //Colour to clear to (AARRGGBB)
		1.0f,  //Value to clear ZBuffer to, doesn't matter since we don't have one
		0 ));   //Stencil clear value, again, we don't have one, this value doesn't matter
}