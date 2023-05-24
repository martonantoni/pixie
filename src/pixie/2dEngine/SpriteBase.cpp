#include "StdAfx.h"

int cSpriteBase::mDebugIDCounter=0;

cSpriteBase::cSpriteBase()
: mDebugID(++mDebugIDCounter)
{
	mProperties.mColor.SetRGBColor(~0);
}

cSpriteBase::~cSpriteBase()
{
	if(mWindow&&IsVisible())
		mWindow->RemoveSprite(this);
}

void cSpriteBase::Drop()
{
	mIsOwned=false;
	if(!mWindow||!IsVisible())
		delete this;
}

void cSpriteBase::Destroy()
{
	delete this;
}

bool cSpriteBase::IsDestroyable() const
{
	if(mIsOwned)
		return false;
	bool CanDestroy=true;
	const_cast<std::remove_const<decltype(mAnimators2)>::type &>(mAnimators2).ForEach([&CanDestroy](auto &Animator) { if(Animator->GetFlags()&cPixieObjectAnimator::KeepsObjectAlive) CanDestroy=false; });
	return CanDestroy;
}

cSpriteBase::eDestroyZombieResult cSpriteBase::DestroyZombie()
{
	if(IsDestroyable())
	{
		mWindow=nullptr; // this is valid ... as long as we are called from cPixieWindow::CheckOwnerlessSprites
		delete this;
		return eDestroyZombieResult::Destroyed;
	}
	return eDestroyZombieResult::StillAlive;
}

void cSpriteBase::SetPosition(cPoint Position)
{
	if(!CheckIfChangableProperty(Property_Position))
		return;
	mProperties.mRect.SetPosition(Position);
	PropertiesSet(Property_Position);
}

void cSpriteBase::SetRect(const cRect &Rect)
{
	if(!CheckIfChangableProperty(Property_Rect))
		return;
	mProperties.mRect=Rect;
	PropertiesSet(Property_Rect);
}

void cSpriteBase::SetSize(cPoint Size)
{
	if(!CheckIfChangableProperty(Property_Size))
		return;
	mProperties.mRect.SetSize(Size);
	PropertiesSet(Property_Size);
}

void cSpriteBase::SetRotation(float Rotation)
{
	if(!CheckIfChangableProperty(Property_Rotation))
		return;
// 	if(Rotation>360)
// 		Rotation%=360;
// 	else if(Rotation<0)
// 		Rotation=360+(Rotation%360);
	mProperties.mRotation=Rotation;
	PropertiesSet(Property_Rotation);
}

void cSpriteBase::SetPositionOffset(const cPoint &PositionOffset)
{
	if(!CheckIfChangableProperty(Property_PositionOffset))
		return;
	mProperties.mPositionOffset=PositionOffset;
	PropertiesSet(Property_PositionOffset);
}

void cSpriteBase::SetRGBColor(cColor Color)
{
	if(!CheckIfChangableProperty(Property_Color))
		return;
	mProperties.mColor.SetRGBColor(Color.GetRGBColor());
	mIsColorSet=true;
	PropertiesSet(Property_Color);
}

void cSpriteBase::SetARGBColor(D3DCOLOR Color)
{
	if(!CheckIfChangableProperty(Property_Color|Property_Alpha))
		return;
	mProperties.mColor.SetRGBColor(Color);
	mIsColorSet=true;
	PropertiesSet(Property_Color|Property_Alpha);
}

void cSpriteBase::SetARGBColor_ByPosition(unsigned int PositionFlags,D3DCOLOR Color)
{
	if(!CheckIfChangableProperty(Property_Color|Property_Alpha))
		return;
	mIsColorSet=true;
	mProperties.mColor.SetARGBColor_ByPosition(PositionFlags,Color);
	PropertiesSet(Property_Color|Property_Alpha);
}

void cSpriteBase::SetAlpha(DWORD Alpha)
{
	if(!CheckIfChangableProperty(Property_Alpha))
		return;
	mProperties.mColor.SetAlpha(Alpha);
	PropertiesSet(Property_Alpha);
}

void cSpriteBase::SetZOrder(int ZOrder)
{
	if(!CheckIfChangableProperty(Property_ZOrder))
		return;
	if(mProperties.mZOrder==ZOrder)
		return;
	if(mWindow&&IsVisible())
		mWindow->RemoveSprite(this);
	mProperties.mZOrder=ZOrder;
	if(mWindow&&IsVisible())
		mWindow->AddSprite(this);
	PropertiesSet(Property_ZOrder);
}

void cSpriteBase::Show()
{
	if(!CheckIfChangableProperty(Property_Visibility))
		return;
	if(mProperties.mVisible)
		return;
	mProperties.mVisible=true;
	if(mWindow)
		mWindow->AddSprite(this);
	PropertiesSet(Property_Visibility);
}

void cSpriteBase::Hide()
{
	if(!CheckIfChangableProperty(Property_Visibility))
		return;
	if(!mProperties.mVisible)
		return;
	mProperties.mVisible=false;
	if(mWindow)
		mWindow->RemoveSprite(this);
	PropertiesSet(Property_Visibility);
}

void cSpriteBase::SetVisible(bool IsVisible)
{
	if(IsVisible)
		Show();
	else
		Hide();
}

void cSpriteBase::SetWindow(cPixieWindow *Window)
{
	if(!CheckIfChangableProperty(Property_Window))
		return;
	if(mWindow&&IsVisible())
		mWindow->RemoveSprite(this);
	mWindow=Window;
	if(mWindow&&IsVisible())
		mWindow->AddSprite(this);
	PropertiesChanged(Property_Window);
}

cRect cSpriteBase::GetRectForRendering() const
{
	cRect RectForRendering(mProperties.mRect);
	RectForRendering.mLeft+=mProperties.mPositionOffset.x;
	RectForRendering.mTop+=mProperties.mPositionOffset.y;
	if(mWindow)
	{
		cRect WindowRect=mWindow->GetScreenRect();
		RectForRendering.mLeft+=WindowRect.mLeft;
		RectForRendering.mTop+=WindowRect.mTop;
	}
	return RectForRendering;
}

cPoint cSpriteBase::GetCenter() const
{
	return GetRect().GetCenter();
}

cPoint cSpriteBase::GetScreenPosition() const
{
	cPixieWindow *Window=GetWindow();
	if(Window)
		return Window->GetScreenRect().GetPosition()+GetPosition();
	else
		return GetPosition();
}

void cSpriteBase::SetCenter(cPoint Center)
{
	cPoint Size=GetSize();
	SetPosition({ Center.x-Size.x/2, Center.y-Size.y/2 });
}

void cSpriteBase::SetValidRect(const cRect &ValidRect)
{
	mValidRect=ValidRect;
	if(ValidRect.mWidth<0||ValidRect.mHeight<0)
	{
		mUseClipping=false;
	}
	else
	{
		mUseClipping=true;
	}
	PropertiesChanged(Property_ValidRect);
}

void cSpriteBase::DisableClipping()
{
	SetValidRect({ 0,0,-1,-1 });
}

cRect cSpriteBase::GetCenterAndHSize() const
{
	return { GetRect().GetCenter(), GetSize()/2 };
}

void cSpriteBase::SetCenterAndHSize(const cRect &Rect)
{
	SetRect(cRect::FromCenterAndSize(Rect.GetPosition(), Rect.GetSize()*2));
}

void cSpriteBase::SetScreenPosition(cPoint Position)
{
	cPixieWindow *Window=GetWindow();
	if(!Window)
		SetPosition(Position);
	else
		SetPosition(Position-Window->GetScreenRect().GetPosition());
}

bool cSpriteBase::GetProperty(unsigned int PropertyFlags,OUT cPropertyValues &PropertyValues) const
{
	switch(PropertyFlags)
	{
	case Property_X: PropertyValues=GetX(); return true;
	case Property_Y: PropertyValues=GetY(); return true;
	case Property_W: PropertyValues=GetWidth(); return true;
	case Property_H: PropertyValues=GetHeight(); return true;
	case Property_Rotation: PropertyValues=GetRotation(); return true;
// 	case Property_XOffset: PropertyValues=GetPositionOffset().x; return true;
// 	case Property_YOffset: PropertyValues=GetPositionOffset().y; return true;
	case Property_ZOrder: PropertyValues=GetZOrder(); return true;
	case Property_Alpha: PropertyValues=GetAlpha(); return true;
	case Property_Position: PropertyValues=GetPosition(); return true;
	case Property_ScreenPosition: PropertyValues=GetScreenPosition(); return true;
	case Property_PositionOffset: PropertyValues=GetPositionOffset(); return true;
	case Property_Size: PropertyValues=GetSize(); return true;
	case Property_Color: PropertyValues=GetColor(); return true;
	case Property_Rect: PropertyValues=GetRect(); return true;
	case Property_CenterAndHSize: PropertyValues=GetCenterAndHSize(); return true;
	case Property_Center: PropertyValues=GetCenter(); return true;
	case Property_ValidRect: PropertyValues=GetValidRect(); return true;
	}
	ASSERT(false);
	return false;
}

bool cSpriteBase::SetProperty(unsigned int PropertyFlags,const cPropertyValues &Value)
{
	if(!CheckIfChangableProperty(PropertyFlags))
		return false;
	switch(PropertyFlags)
	{
	case Property_X: SetPosition(Value.ToInt(),GetY()); return true;
	case Property_Y: SetPosition(GetX(),Value.ToInt()); return true;
	case Property_W: SetSize(Value.ToInt(),GetHeight()); return true;
	case Property_H: SetSize(GetWidth(),Value.ToInt()); return true;
	case Property_Rotation: SetRotation(Value.ToInt()); return true;
// 	case Property_XOffset: SetPositionOffset(Value.ToInt(), GetPositionOffset().y); return true;
// 	case Property_YOffset: SetPositionOffset(GetPositionOffset().x, Value.ToInt()); return true;
	case Property_ZOrder: SetZOrder(Value.ToInt()); return true;
	case Property_Alpha: SetAlpha(Value.ToInt()); return true;
	case Property_Position: SetPosition(Value.ToPoint()); return true;
	case Property_ScreenPosition: SetScreenPosition(Value.ToPoint()); return true;
	case Property_PositionOffset: SetPositionOffset(Value.ToPoint()); return true;
	case Property_CenterAndHSize: SetCenterAndHSize(Value.ToRect()); return true;
	case Property_Center: SetCenter(Value.ToPoint()); return true;
	case Property_Size: SetSize(Value.ToPoint()); return true;
	case Property_Color: SetRGBColor(Value.ToRGBColor()); return true;
	case Property_Rect: SetRect(Value.ToRect()); return true;
	case Property_ValidRect: SetValidRect(Value.ToRect()); return true;
	}
	ASSERT(false);
	return false;
}

bool cSpriteBase::GetFloatProperty(unsigned int PropertyFlags, OUT float &Value) const
{
	switch(PropertyFlags)
	{
	case Property_Rotation: Value=GetRotation(); return true;
	}
	ASSERT(false);
	return false;
}

bool cSpriteBase::SetFloatProperty(unsigned int PropertyFlags, float Value)
{
	if(!CheckIfChangableProperty(PropertyFlags))
		return false;
	switch(PropertyFlags)
	{
	case Property_Rotation: SetRotation(Value); return true;
	}
	ASSERT(false);
	return false;
}

bool cSpriteBase::SetStringProperty(unsigned int PropertyFlags, const std::string &Value)
{
	if(!CheckIfChangableProperty(PropertyFlags))
		return false;
	ASSERT(false);
	return false;
}

void cSpriteBase::CopyProperties(const cSpriteBase& source)
{
    mProperties = source.mProperties;
    mValidRect = source.mValidRect;
    mIsColorSet = source.mIsColorSet;
    mUseClipping = source.mUseClipping;
    mWindow = source.mWindow;
}
