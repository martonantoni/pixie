#pragma once

class cRenderer;
class cDeviceClearer;
class cTexture;

class cDevice
{
	IDirect3DDevice9 *mDevice=nullptr;
	IDirect3DSurface9 *mBackBufferSurface=nullptr;
	D3DPRESENT_PARAMETERS mPresentParameters;
	volatile int mIsClosing=false;
	typedef std::list<cRenderer *> cRendererList;
	cRendererList mRenderers;
	cRenderer *mMainRenderer=nullptr;
	cDeviceClearer *mDeviceClearer=nullptr;
	bool mNeedClear=false;
	void Init();
	void Close();
	cRegisteredID mRenderingTimerID;
	void RenderingLoop();
	cDevice();
public:
	~cDevice();
	static cDevice *Get();
	operator IDirect3DDevice9 *() const { return mDevice; }
	IDirect3DDevice9 *GetD3DObject() { return mDevice; }
	
	void SetClearer(cDeviceClearer *pDeviceClearer);
	void AddRenderer(cRenderer *Renderer);
	void RemoveRenderer(cRenderer *Renderer);
	void AddMainRenderer(cRenderer *MainRenderer);
	const D3DPRESENT_PARAMETERS &GetPresentParameters() const { return mPresentParameters; }
};

extern cDevice *theDevice;
