#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
HINSTANCE gInstance;
cPrimaryWindow *thePrimaryWindow=NULL;

LRESULT CALLBACK MainWindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	cWindowsMessageResult Result=thePrimaryWindow->WindowProc(uMsg,wParam,lParam);
	return Result.mIsValid?Result.mResult:DefWindowProc(hwnd,uMsg,wParam,lParam);
}

bool g_IsExitActive=false;

cWindowsMessageResult cPrimaryWindow::WindowProc(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	cMessageMap::iterator i=mMessageMap.find(uMsg);
	if(i!=mMessageMap.end())
	{
		cMessageHandlers &MessageHandler=i->second;
		for(cMessageHandlers::iterator j=MessageHandler.begin(),jend=MessageHandler.end();j!=jend;++j)
		{
			cWindowsMessageResult Result=j->first(wParam,lParam);
			if(Result.mIsValid)
				return Result;
		}
	}
	// exit hack:
	if(uMsg==WM_DESTROY)
	{
		MainLog->Flush();
		g_IsExitActive=true;
		exit(0);
	}
	return cWindowsMessageResult();
}

cPrimaryWindow::cPrimaryWindow()
{
	thePrimaryWindow=this;
	

	WNDCLASSEX WindowClass;
	memset(&WindowClass,0,sizeof(WindowClass));
	WindowClass.cbSize=sizeof(WindowClass);
	WindowClass.style=CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
	WindowClass.lpfnWndProc=&MainWindowProc;
	WindowClass.hInstance=gInstance;
	WindowClass.lpszClassName="PixieWindow";
#ifdef NO_CUSTOM_CURSORS
	WindowClass.hCursor=LoadCursor(NULL, IDC_ARROW);
#else
	WindowClass.hCursor=NULL;// CursorHandle;// LoadCursor(NULL, IDC_ARROW);
#endif

	::RegisterClassEx(&WindowClass);
	auto config = theGlobalConfig->getSubConfig("primary_window");
	mWindowHandle=::CreateWindow("PixieWindow",cProgramTitle::Get()->c_str(),WS_VISIBLE|WS_SYSMENU|WS_MINIMIZEBOX,0,0,500,500,NULL,NULL,gInstance,0);

	RECT WindowRect,ClientRect;
	::GetWindowRect(mWindowHandle,&WindowRect);
	::GetClientRect(mWindowHandle,&ClientRect);

	int ScreenWidth=::GetSystemMetrics(SM_CXSCREEN);

	int Width= config->get<int>("width")+WindowRect.right-WindowRect.left-ClientRect.right;
	int Height= config->get<int>("height")+WindowRect.bottom-WindowRect.top-ClientRect.bottom;

	::SetWindowPos(mWindowHandle,NULL,(ScreenWidth-Width)/2,0,Width,Height,SWP_NOZORDER);
    ::BringWindowToTop(mWindowHandle);

//	AddMessageHandler(WM_NCHITTEST,MakeFunctor(this,&cPrimaryWindow::OnNCHitTest));
}

cRegisteredID cPrimaryWindow::AddMessageHandler(UINT Message,const cMessageHandler &MessageHandler)
{
	cIDData *IDData=new cIDData;
	IDData->mMessage=Message;
	IDData->mID=++mIDCounter;
	mMessageMap[Message].emplace_back(MessageHandler,mIDCounter);
	return cRegisteredID(this,IDData);
}

void cPrimaryWindow::Unregister(const cRegisteredID &RegisteredID,eCallbackType CallbackType)
{
	cIDData *IDData=(cIDData *)RegisteredID.GetIDData();
	auto &MessageHandlers=mMessageMap[IDData->mMessage];
	auto i=std::find_if(MessageHandlers, [ID=IDData->mID](auto &item){ return item.second==ID; });
	if(ASSERTTRUE(i!=MessageHandlers.end()))
		MessageHandlers.erase(i);
	delete IDData;
}

cWindowsMessageResult cPrimaryWindow::OnNCHitTest(WPARAM wParam,LPARAM lParam)
{
	return cWindowsMessageResult(HTCAPTION);
}

void cPrimaryWindow::Resize(int Width,int Height)
{
	SetWindowPos(mWindowHandle,NULL,0,0,Width,Height,SWP_NOMOVE|SWP_NOZORDER);
}

void cPrimaryWindow::Close()
{
	::PostMessage(mWindowHandle, WM_CLOSE, 0, 0);
}