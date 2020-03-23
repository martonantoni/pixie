#pragma once

class cRenderer abstract
{
protected:
	IDirect3DSurface9 *mRenderSurface;
public:
	virtual ~cRenderer() {}
	virtual void Render() abstract;
	void SetRenderSurface(IDirect3DSurface9 *RenderSurface) { mRenderSurface=RenderSurface; }
};
