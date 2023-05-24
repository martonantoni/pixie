#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cPixieWindow::cPixieWindow()
: mIsValid(false)
, mParentWindow(NULL)
, mWindowSprite(NULL)
, mAlpha(0)
, mIsTrackingMouse(false)
, mIsVisible(true)
, mZIndex(0)
{
}

void cPixieWindow::Init(const cRect &Placement,const cConfig &Config)
{
	mPlacement=Placement;
	InitSelf(Config);
	InitVisualizer(Config);
}

void cPixieWindow::Init(const cConfig &Config)
{
	cRect Placement;
	Placement.mLeft=Config.GetInt("x");
	Placement.mTop=Config.GetInt("y");
	Placement.mWidth=Config.GetInt("w");
	Placement.mHeight=Config.GetInt("h");
	Init(Placement,Config);
}

cPixieWindow::~cPixieWindow()
{
	if(mIsTrackingMouse)
		StopMouseTracking();
	if(mParentWindow)
		mParentWindow->RemoveWindow(this);
	BlendObjectAlpha(mWindowSprite,255,100);
	mWindowSprite->Drop();
// delete any child window still attached:
	while(!mChildWindows.empty())
		delete mChildWindows.front();
	int alma=2;
}

void cPixieWindow::RedrawIfNeeded()
{
	if(mIsValid)
	{
		return;
	}
	Redraw();
	for(cPixieWindows::iterator i=mChildWindows.begin(),iend=mChildWindows.end();i!=iend;++i)
	{
		(*i)->RedrawIfNeeded();
	}
// reallocate / allocate texture if needed:
	if(!mWindowTexture||
		mWindowTexture->GetSurfaceWidth()!=mPlacement.mWidth||mWindowTexture->GetSurfaceHeight()!=mPlacement.mHeight)
	{
		if(mWindowSprite)
			mWindowSprite->Drop();
		mWindowSprite=NULL;
		mWindowTexture.reset();
	}
	if(!mWindowTexture)
	{
		mWindowTexture=cTexture::CreateRenderTarget(mPlacement.mWidth,mPlacement.mHeight);
	}
	if(!mWindowSprite)
	{
		mWindowSprite=new cSprite(mParentWindow);
	}
	mWindowSprite->SetPosition(mPlacement.mLeft,mPlacement.mTop);
	mWindowSprite->SetSize(mPlacement.mWidth,mPlacement.mHeight);
	mWindowSprite->SetZOrder(mZIndex);
	mWindowSprite->SetTexture(mWindowTexture);
	mWindowSprite->SetAlpha(mAlpha);
	if(mIsVisible)
		mWindowSprite->Show();
	else
		mWindowSprite->Hide();
// redraw the texture:
	mWindowTexture->SetAsRenderTarget();
	mRenderer.Render();
	mIsValid=true;
}

void cPixieWindow::Invalidate()
{
	if(mIsValid)
	{
		mIsValid=false;
		if(mParentWindow)
			mParentWindow->Invalidate();
	}
}

void cPixieWindow::Show()
{
	mIsVisible=true;
	if(mWindowSprite)
		mWindowSprite->Show();
	if(mParentWindow)
		mParentWindow->Invalidate();
}

void cPixieWindow::Hide()
{
	mIsVisible=false;
	if(mWindowSprite)
		mWindowSprite->Hide();
	if(mParentWindow)
		mParentWindow->Invalidate();
}

void cPixieWindow::ChildZOrderChanged(cPixieWindow *Window)
{
	mChildWindows.remove(Window);
	InsertWindowToChildList(Window);
}

void cPixieWindow::InsertWindowToChildList(cPixieWindow *Window)
{
// 	for(cPixieWindows::iterator i=mChildWindows.begin(),iend=mChildWindows.end();;++i)
// 	{
// 		if(i==iend||(*i)->mPlacement.mZOrder>Window->mPlacement.mZOrder)
// 		{
// 			mChildWindows.insert(i,Window);
// 			return;
// 		}
// 	}
	mChildWindows.push_back(Window);
	Window->mZIndex=int(mChildWindows.size());
}

void cPixieWindow::AddWindow(cPixieWindow *Window)
{
	ASSERT(Window->mParentWindow==NULL);
	Window->mParentWindow=this;
	InsertWindowToChildList(Window);
	mIsValid=false;
}

void cPixieWindow::RemoveWindow(cPixieWindow *Window)
{
	ASSERT(Window->mParentWindow==this);
	Window->mParentWindow=NULL;
	mChildWindows.remove(Window);
	mIsValid=false;
}

void cPixieWindow::SetPlacement(const cRect &Placement)
{
	mPlacement=Placement;
	Invalidate();
}

cPixieWindow *cPixieWindow::GetWindowAt(INOUT int &x,INOUT int &y)
{
	cPixieWindow *Window=this;
	for(;;)
	{
		// the window list is ordered in the order they need to be drawn: the last in the list is the one drawn last: that is on top of the others
		for(cPixieWindows::const_reverse_iterator i=Window->mChildWindows.rbegin(),iend=Window->mChildWindows.rend();;++i)
		{
			if(i==iend)
				return Window;
			cPixieWindow *TestedWindow=*i;
			int RelativeX=x-TestedWindow->mPlacement.mLeft,RelativeY=y-TestedWindow->mPlacement.mTop;
			if(TestedWindow->HitTest(cPoint(RelativeX,RelativeY))==Hit_Inside)
			{
				Window=TestedWindow;
				x=RelativeX;
				y=RelativeY;
				break;
			}
		}
	}
}

cPixieWindow *cPixieWindow::GetWindowAt(const cPoint &Coordinates) 
{
	int x=Coordinates.x;
	int y=Coordinates.y;
	return GetWindowAt(x,y);
}

cPoint cPixieWindow::ScreenCoordinatesToWindowCoordinates(const cPoint &Point) const
{
	cPoint WindowCoordinates(Point);
	for(const cPixieWindow *Window=this;Window;Window=Window->mParentWindow)
	{
		WindowCoordinates.x-=Window->mPlacement.mLeft;
		WindowCoordinates.y-=Window->mPlacement.mTop;
	}
	return WindowCoordinates;
}

cPoint cPixieWindow::WindowCoordinatesToScreenCoordinates(const cPoint &Point) const
{
	cPoint ScreenCoordinates(Point);
	for(const cPixieWindow *Window=this;Window;Window=Window->mParentWindow)
	{
		ScreenCoordinates.x+=Window->mPlacement.mLeft;
		ScreenCoordinates.y+=Window->mPlacement.mTop;
	}
	return ScreenCoordinates;
}

cPixieWindow::eHitTestResult cPixieWindow::HitTest(const cPoint &WindowRelativeCoordinates) const
{
	return WindowRelativeCoordinates.x>=0&&WindowRelativeCoordinates.x<mPlacement.mWidth&&
		WindowRelativeCoordinates.y>=0&&WindowRelativeCoordinates.y<mPlacement.mHeight ? Hit_Inside:Hit_Outsite;
}

void cPixieWindow::SetAlpha(DWORD Alpha)
{
	mAlpha=Alpha;
	if(mWindowSprite)
		mWindowSprite->SetAlpha(mAlpha);
}

void cPixieWindow::StartMouseTracking()
{
	mIsTrackingMouse=true;
	thePixieDesktop.StartMouseTracking(this);
}

void cPixieWindow::StopMouseTracking()
{
	mIsTrackingMouse=false;
	thePixieDesktop.StopMouseTracking(this);
}