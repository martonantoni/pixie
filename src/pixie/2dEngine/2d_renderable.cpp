#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

int c2DRenderable::mDebugIDCounter=0;

c2DRenderable::c2DRenderable()
: mDebugID(++mDebugIDCounter)
{
	mProperties.mColor.SetRGBColor(~0);
}

c2DRenderable::~c2DRenderable()
{
	if(mWindow&&IsVisible())
		mWindow->RemoveSprite(this);
}

void c2DRenderable::Drop()
{
	mIsOwned=false;
	if(!mWindow||!IsVisible())
		delete this;
}

void c2DRenderable::Destroy()
{
	delete this;
}

bool c2DRenderable::IsDestroyable() const
{
	if(mIsOwned)
		return false;
	bool CanDestroy=true;
	const_cast<std::remove_const<decltype(mAnimators2)>::type &>(mAnimators2).ForEach([&CanDestroy](auto &Animator) { if(Animator->GetFlags()&cPixieObjectAnimator::KeepsObjectAlive) CanDestroy=false; });
	return CanDestroy;
}

c2DRenderable::eDestroyZombieResult c2DRenderable::DestroyZombie()
{
	if(IsDestroyable())
	{
		mWindow=nullptr; // this is valid ... as long as we are called from cPixieWindow::CheckOwnerlessSprites
		delete this;
		return eDestroyZombieResult::Destroyed;
	}
	return eDestroyZombieResult::StillAlive;
}


void c2DRenderable::SetRotation(float Rotation)
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

void c2DRenderable::SetRGBColor(cColor Color)
{
	if(!CheckIfChangableProperty(Property_Color))
		return;
	mProperties.mColor.SetRGBColor(Color.GetRGBColor());
	mIsColorSet=true;
	PropertiesSet(Property_Color);
}

void c2DRenderable::SetARGBColor(D3DCOLOR Color)
{
	if(!CheckIfChangableProperty(Property_Color|Property_Alpha))
		return;
	mProperties.mColor.SetRGBColor(Color);
	mIsColorSet=true;
	PropertiesSet(Property_Color|Property_Alpha);
}

void c2DRenderable::SetARGBColor_ByPosition(unsigned int PositionFlags,D3DCOLOR Color)
{
	if(!CheckIfChangableProperty(Property_Color|Property_Alpha))
		return;
	mIsColorSet=true;
	mProperties.mColor.SetARGBColor_ByPosition(PositionFlags,Color);
	PropertiesSet(Property_Color|Property_Alpha);
}

void c2DRenderable::SetAlpha(DWORD Alpha)
{
	if(!CheckIfChangableProperty(Property_Alpha))
		return;
	mProperties.mColor.SetAlpha(Alpha);
	PropertiesSet(Property_Alpha);
}

void c2DRenderable::SetZOrder(int ZOrder)
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

void c2DRenderable::Show()
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

void c2DRenderable::Hide()
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

void c2DRenderable::SetVisible(bool IsVisible)
{
	if(IsVisible)
		Show();
	else
		Hide();
}

void c2DRenderable::SetWindow(cPixieWindow *Window)
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

void c2DRenderable::SetValidRect(const cRect &ValidRect)
{
	mProperties.mValidRect=ValidRect;
	if (ValidRect.width() >= 0 && ValidRect.height() >= 0 && mProperties.mClippingMode == eClippingMode::None) // backward compatibility
	{
		mProperties.mClippingMode = eClippingMode::Parent;
		PropertiesChanged(Property_ClippingMode);
	}
	PropertiesChanged(Property_ValidRect);
}

void c2DRenderable::DisableClipping()
{
	mProperties.mClippingMode = eClippingMode::None;
	PropertiesChanged(Property_ClippingMode);
}

bool c2DRenderable::GetProperty(unsigned int PropertyFlags,OUT cPropertyValues &PropertyValues) const
{
	switch(PropertyFlags)
	{
	case Property_Rotation: PropertyValues=GetRotation(); return true;
	case Property_ZOrder: PropertyValues=GetZOrder(); return true;
	case Property_Alpha: PropertyValues=GetAlpha(); return true;
	case Property_Color: PropertyValues=GetColor(); return true;
	case Property_ValidRect: PropertyValues=GetValidRect(); return true;
	}
	ASSERT(false);
	return false;
}

bool c2DRenderable::SetProperty(unsigned int PropertyFlags,const cPropertyValues &Value)
{
	if(!CheckIfChangableProperty(PropertyFlags))
		return false;
	switch(PropertyFlags)
	{
	case Property_Rotation: SetRotation(Value.ToInt()); return true;
	case Property_ZOrder: SetZOrder(Value.ToInt()); return true;
	case Property_Alpha: SetAlpha(Value.ToInt()); return true;
	case Property_Color: SetRGBColor(Value.ToRGBColor()); return true;
	case Property_ValidRect: SetValidRect(Value.ToRect()); return true;
	}
	ASSERT(false);
	return false;
}

bool c2DRenderable::GetFloatProperty(unsigned int PropertyFlags, OUT float &Value) const
{
	switch(PropertyFlags)
	{
	case Property_Rotation: Value=GetRotation(); return true;
	}
	ASSERT(false);
	return false;
}

bool c2DRenderable::SetFloatProperty(unsigned int PropertyFlags, float Value)
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

bool c2DRenderable::SetStringProperty(unsigned int PropertyFlags, const std::string &Value)
{
	if(!CheckIfChangableProperty(PropertyFlags))
		return false;
	ASSERT(false);
	return false;
}

void c2DRenderable::CopyProperties(const c2DRenderable& source)
{
    mProperties = source.mProperties;
    mIsColorSet = source.mIsColorSet;
    mWindow = source.mWindow;
	// visibility is tricky: when the properties are copied, mVisibility is also copied, but that
	// does not mean that the sprite is visible. It is only visible if it is added to a window.
	//
	// check if visible, add to window
	if (mWindow && mProperties.mVisible)
        mWindow->AddSprite(this);
}

void c2DRenderable::setClippingMode(eClippingMode ClippingMode)
{
    mProperties.mClippingMode = ClippingMode;
	PropertiesChanged(Property_ClippingMode);
}
