#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

cDevice *theDevice=NULL;


cDevice::cDevice()
: mBackBufferSurface(NULL)
, mDevice(NULL)
{
	mIsClosing=false;
	theDevice=this;
}

cDevice::~cDevice()
{
	theDevice=NULL;
}

void cDevice::Init()
{


	IDirect3D9 *Direct3D=Direct3DCreate9(D3D_SDK_VERSION);
	RELEASE_ASSERT_EXT(Direct3D,"Direct3DCreate9 failed");

	D3DFORMAT format=D3DFMT_A8R8G8B8;// D3DFMT_R5G6B5; //For simplicity we'll hard-code this for now.

	//Even though we set all of it's members, it's still good practice to zero it out
	ZeroMemory(&mPresentParameters,sizeof(D3DPRESENT_PARAMETERS));

	mPresentParameters.BackBufferCount= 1;  //We only need a single back buffer
	mPresentParameters.MultiSampleType=D3DMULTISAMPLE_NONE; //No multi-sampling
	mPresentParameters.MultiSampleQuality=0;                //No multi-sampling
	mPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;  // Throw away previous frames, we don't need them
//	mPresentParameters.hDeviceWindow=cPrimaryWindow::Get()->m_hWnd;
	mPresentParameters.Flags=0;            //No flags to set
	mPresentParameters.FullScreen_RefreshRateInHz=D3DPRESENT_RATE_DEFAULT; //Default Refresh Rate
	mPresentParameters.PresentationInterval=D3DPRESENT_INTERVAL_DEFAULT;   //Default Presentation rate
	mPresentParameters.BackBufferFormat=format;      //Display format
	mPresentParameters.EnableAutoDepthStencil=FALSE; //No depth/stencil buffer

	/*	if(is_app_fullscreen){
	mPresentParameters.Windowed          = FALSE;
	mPresentParameters.BackBufferWidth   = 640;
	mPresentParameters.BackBufferHeight  = 480;
	}else{*/
 	//mPresentParameters.BackBufferWidth   = 3840/2;
 	//mPresentParameters.BackBufferHeight  = 2160/2;
	mPresentParameters.Windowed          = TRUE;


// 	mPresentParameters.Windowed          = FALSE;
// 	mPresentParameters.BackBufferWidth   = 1680;
// 	mPresentParameters.BackBufferHeight  = 1050;
// 

	//	}


	// 	const D3DPRESENT_PARAMETERS &D3DPresentParams=mPixieDevice->GetPresenetParameters();
	// 	float BackBufferWidthHalf=(float)(D3DPresentParams.BackBufferWidth/2);
	// 	float BackBufferHeightHalf=(float)(D3DPresentParams.BackBufferHeight/2);

	cPrimaryWindow &PrimaryWindow=cPrimaryWindow::Get();

	StopOnError(Direct3D->CreateDevice(D3DADAPTER_DEFAULT, //The default adapter, on a multi-monitor system
		//there can be more than one.
		D3DDEVTYPE_HAL, //Use hardware acceleration rather than the software renderer
		//Our Window
		PrimaryWindow.mWindowHandle,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		//Our D3DPRESENT_PARAMETERS structure, so it knows what we want to build
		&mPresentParameters,
		//This will be set to point to the new device
		&mDevice));

	mDevice->GetRenderTarget(0,&mBackBufferSurface);

	mRenderingTimerID=theMainThread->AddTimer([this]() { RenderingLoop(); }, cTimerRequest(10));

	mNeedClear= theGlobalConfig->get<bool>("pixie_system.clear_device_before_frame", false);
}

void cDevice::Close()
{
	mIsClosing=true;
//	mDevice->Reset();
}

void cDevice::RenderingLoop()
{
	if(mIsClosing)
		return;
	StopOnError(mDevice->BeginScene());

	theLogicServer.Tick();

	theRenderers.Call();

//	StopOnError(mDevice->SetRenderTarget(0,mBackBufferSurface));

	if(mNeedClear&&mDeviceClearer)
		mDeviceClearer->ClearDevice(mDevice);

	for(cRendererList::iterator i=mRenderers.begin(),iend=mRenderers.end();i!=iend;++i)
		(*i)->Render();

	if(mMainRenderer)
		mMainRenderer->Render();

	StopOnError(mDevice->EndScene());

	//Show the results
	StopOnError(mDevice->Present(NULL,  //Source rectangle to display, NULL for all of it
		NULL,  //Destination rectangle, NULL to fill whole display
		NULL,  //Target window, if NULL uses device window set in CreateDevice
		NULL ));//Unused parameter, set it to NULL
}

cDevice *cDevice::Get()
{
	static cDevice *Instance=NULL;
	if(!Instance)
	{
		Instance=new cDevice;
		theMainThread->callback([]() {Instance->Init(); }, eCallbackType::Wait);
	}
	return Instance;
}

void cDevice::SetClearer(cDeviceClearer *pDeviceClearer)
{
	mDeviceClearer=pDeviceClearer;
}

void cDevice::AddRenderer(cRenderer *Renderer)
{
	mRenderers.push_back(Renderer);
}

void cDevice::RemoveRenderer(cRenderer *Renderer)
{
	if(mMainRenderer==Renderer)
	{
		mMainRenderer=nullptr;
		return;
	}
	mRenderers.remove(Renderer);
}

void cDevice::AddMainRenderer(cRenderer *MainRenderer)
{
	mMainRenderer=MainRenderer;
	mMainRenderer->SetRenderSurface(mBackBufferSurface);
}
