#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
HINSTANCE gInstance;
cPrimaryWindow* thePrimaryWindow = NULL;

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	cWindowsMessageResult Result = thePrimaryWindow->WindowProc(uMsg, wParam, lParam);
	return Result.mIsValid ? Result.mResult : DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool g_IsExitActive = false;

cWindowsMessageResult cPrimaryWindow::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	cMessageMap::iterator i = mMessageMap.find(uMsg);
	if (i != mMessageMap.end())
	{
		cMessageHandlers& MessageHandler = i->second;
		for (cMessageHandlers::iterator j = MessageHandler.begin(), jend = MessageHandler.end(); j != jend; ++j)
		{
			cWindowsMessageResult Result = j->first(wParam, lParam);
			if (Result.mIsValid)
				return Result;
		}
	}
	// exit hack:
	if (uMsg == WM_DESTROY)
	{
        mNCTestID.Unregister();
		MainLog->Flush();
		g_IsExitActive = true;
		exit(0);
	}
	return cWindowsMessageResult();
}

cPrimaryWindow::cPrimaryWindow()
{
    thePrimaryWindow = this;

    WNDCLASSEX WindowClass;
    memset(&WindowClass, 0, sizeof(WindowClass));
    WindowClass.cbSize = sizeof(WindowClass);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    WindowClass.lpfnWndProc = &MainWindowProc;
    WindowClass.hInstance = gInstance;
    WindowClass.lpszClassName = "PixieWindow";

#ifdef NO_CUSTOM_CURSORS
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
#else
    WindowClass.hCursor = NULL;
#endif

    ::RegisterClassEx(&WindowClass);

    auto config = theGlobalConfig->getSubConfig("primary_window");

    bool borderless = config->get<bool>("borderless", false);
    int Width = config->get<int>("width");
    int Height = config->get<int>("height");

    DWORD windowStyle = borderless
        ? WS_POPUP | WS_VISIBLE
        : WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX;

    mWindowHandle = ::CreateWindow(
        "PixieWindow",
        cProgramTitle::Get()->c_str(),
        windowStyle,
        0, 0, 500, 500,
        NULL, NULL, gInstance, 0);


    if (!borderless)
    {
        RECT WindowRect, ClientRect;
        ::GetWindowRect(mWindowHandle, &WindowRect);
        ::GetClientRect(mWindowHandle, &ClientRect);

        Width += WindowRect.right - WindowRect.left - ClientRect.right;
        Height += WindowRect.bottom - WindowRect.top - ClientRect.bottom;
    }

    int ScreenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int ScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    ::SetWindowPos(mWindowHandle, NULL, 
        (ScreenWidth - Width) / 2, (ScreenHeight - Height) / 2,
        Width, Height, SWP_NOZORDER);

    ::BringWindowToTop(mWindowHandle);

    if(borderless)
        mNCTestID = AddMessageHandler(WM_NCHITTEST, [this](WPARAM wParam, LPARAM lParam) { return OnNCHitTest(wParam, lParam); });
}

cRegisteredID cPrimaryWindow::AddMessageHandler(UINT Message, const cMessageHandler& MessageHandler)
{
	cIDData* IDData = new cIDData;
	IDData->mMessage = Message;
	IDData->mID = ++mIDCounter;
	mMessageMap[Message].emplace_back(MessageHandler, mIDCounter);
	return cRegisteredID(this, IDData);
}

void cPrimaryWindow::Unregister(const cRegisteredID& RegisteredID, eCallbackType CallbackType)
{
	cIDData* IDData = (cIDData*)RegisteredID.GetIDData();
	auto& MessageHandlers = mMessageMap[IDData->mMessage];
	auto i = std::ranges::find_if(MessageHandlers, [ID = IDData->mID](auto& item) { return item.second == ID; });
	if (ASSERTTRUE(i != MessageHandlers.end()))
		MessageHandlers.erase(i);
	delete IDData;
}

cWindowsMessageResult cPrimaryWindow::OnNCHitTest(WPARAM wParam, LPARAM lParam)
{
    if (::GetKeyState(VK_MENU) & 0x8000)
        return cWindowsMessageResult(HTCAPTION);
    return cWindowsMessageResult(HTCLIENT);
}

void cPrimaryWindow::Resize(int Width, int Height)
{
	SetWindowPos(mWindowHandle, NULL, 0, 0, Width, Height, SWP_NOMOVE | SWP_NOZORDER);
}

void cPrimaryWindow::Close()
{
	::PostMessage(mWindowHandle, WM_CLOSE, 0, 0);
}