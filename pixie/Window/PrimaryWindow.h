#pragma once

extern HINSTANCE gInstance;

struct cWindowsMessageResult
{
	LRESULT mResult;
	BOOL mIsValid;
	cWindowsMessageResult(): mIsValid(false) {}
	explicit cWindowsMessageResult(LRESULT Result): mIsValid(true), mResult(Result) {}
};

class cPrimaryWindow: public cRegistrationHandler, public tSingleton<cPrimaryWindow>
{
public:
	typedef std::function<cWindowsMessageResult(WPARAM wParam, LPARAM lParam)> cMessageHandler;
private:
	typedef std::list<std::pair<cMessageHandler,size_t>> cMessageHandlers;
	typedef std::map<UINT,cMessageHandlers> cMessageMap;
	cMessageMap mMessageMap;
	struct cIDData
	{
		UINT mMessage;
		size_t mID;
	};
	size_t mIDCounter=0;
	cWindowsMessageResult OnNCHitTest(WPARAM wParam,LPARAM lParam);
public:
	cPrimaryWindow();
	HWND mWindowHandle;
	cRegisteredID AddMessageHandler(UINT Message,const cMessageHandler &MessageHandler);
	virtual void Unregister(const cRegisteredID &RegisteredID,eCallbackType CallbackType=eCallbackType::Wait) override;
	void Resize(int Width,int Height);
	void Close();

	cWindowsMessageResult WindowProc(UINT uMsg,WPARAM wParam,LPARAM lParam);
};