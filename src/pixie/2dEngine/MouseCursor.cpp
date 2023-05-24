#include "StdAfx.h"

cMouseCursor::cMouseCursor()
	: mMoveListener("pixie.mouse.move", [this](const auto &Event) {OnMouseMove(Event); })
{
	mDefaultCursorTexture=theTextureManager.GetTexture("cursor_arrow");
	mSprite=std::make_unique<cSprite>();
	mSprite->SetWindow(&thePixieDesktop.GetTopLayer());
	mSprite->SetTexture(mDefaultCursorTexture);
	mSprite->SetPosition(20,20);
	mSprite->SetSize(32,32);
	mSprite->SetZOrder(1000);
	mSprite->Show();
	theIDSink=cPrimaryWindow::Get().AddMessageHandler(WM_SETCURSOR, [this](auto wp, auto lp) { return OnSetCursor(wp, lp); });
}

cMouseCursor::~cMouseCursor()
{
}

void cMouseCursor::OnMouseMove(const cEvent &Event)
{
	const cPoint *ScreenCoordinates=mPointHolder.GetData(Event.mEventDataID);
	if(ASSERTTRUE(ScreenCoordinates))
		mSprite->SetPosition(*ScreenCoordinates);
}

cWindowsMessageResult cMouseCursor::OnSetCursor(WPARAM wParam,LPARAM lParam)
{
	static BOOL IsVisible=true;
	if(LOWORD(lParam)==HTCLIENT)
	{
		if(IsVisible)
		{
			ShowCursor(false);
			IsVisible=false;
			if(!mIsHidden)
				mSprite->Show();
		}
	}
	else
	{
		if(!IsVisible)
		{
			ShowCursor(true);
			IsVisible=true;
			mSprite->Hide();
		}
	}
	return cWindowsMessageResult(FALSE);
}

void cMouseCursor::SetTexture(tIntrusivePtr<cTexture> Texture)
{
	if(Texture)
		mSprite->SetTexture(Texture);
	else
		mSprite->SetTexture(mDefaultCursorTexture);
}

void cMouseCursor::Show()
{
	mIsHidden=false;
	mSprite->Show();
}

void cMouseCursor::Hide()
{
	mIsHidden=true;
	mSprite->Hide();
}
