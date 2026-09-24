#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

bool cSpriteBase::GetProperty(unsigned int PropertyFlags, OUT cPropertyValues& PropertyValues) const
{
	switch (PropertyFlags)
	{
	case Property_X: PropertyValues = GetX(); return true;
	case Property_Y: PropertyValues = GetY(); return true;
	case Property_W: PropertyValues = GetWidth(); return true;
	case Property_H: PropertyValues = GetHeight(); return true;
	case Property_Position: PropertyValues = GetPosition(); return true;
	case Property_ScreenPosition: PropertyValues = GetScreenPosition(); return true;
	case Property_PositionOffset: PropertyValues = GetPositionOffset(); return true;
	case Property_Size: PropertyValues = GetSize(); return true;
	case Property_Rect: PropertyValues = GetRect(); return true;
	case Property_CenterAndHSize: PropertyValues = GetCenterAndHSize(); return true;
	case Property_Center: PropertyValues = center(); return true;
	case Property_Rotation: PropertyValues = GetRotation(); return true;
	case Property_ZOrder: PropertyValues = GetZOrder(); return true;
	case Property_Alpha: PropertyValues = GetAlpha(); return true;
	case Property_Color: PropertyValues = GetColor(); return true;
	case Property_ShaderParam0: PropertyValues = getShaderParam(0); return true;
	case Property_ShaderParam1: PropertyValues = getShaderParam(1); return true;
	case Property_ShaderParam2: PropertyValues = getShaderParam(2); return true;
	case Property_ShaderParam3: PropertyValues = getShaderParam(3); return true;
	}
	ASSERT(false);
	return false;
}

bool cSpriteBase::SetProperty(unsigned int PropertyFlags, const cPropertyValues& Value)
{
	if (!CheckIfChangableProperty(PropertyFlags))
		return false;
	switch (PropertyFlags)
	{
	case Property_Rotation: SetRotation(Value.ToFloat()); return true;
	case Property_ZOrder: SetZOrder(Value.ToInt()); return true;
	case Property_Alpha: SetAlpha(Value.ToInt()); return true;
	case Property_Color: SetRGBColor(Value.ToRGBColor()); return true;
	case Property_ShaderParam0: setShaderParam(0, Value.ToFloat()); return true;
	case Property_ShaderParam1: setShaderParam(1, Value.ToFloat()); return true;
	case Property_ShaderParam2: setShaderParam(2, Value.ToFloat()); return true;
	case Property_ShaderParam3: setShaderParam(3, Value.ToFloat()); return true;
	case Property_X: SetPosition(Value.ToInt(), GetY()); return true;
	case Property_Y: SetPosition(GetX(), Value.ToInt()); return true;
	case Property_W: SetSize(Value.ToInt(), GetHeight()); return true;
	case Property_H: SetSize(GetWidth(), Value.ToInt()); return true;
	case Property_Position: SetPosition(Value.ToPoint()); return true;
	case Property_ScreenPosition: SetScreenPosition(Value.ToPoint()); return true;
	case Property_PositionOffset: SetPositionOffset(Value.ToPoint()); return true;
	case Property_CenterAndHSize: SetCenterAndHSize(Value.ToRect()); return true;
	case Property_Center: SetCenter(Value.ToPoint()); return true;
	case Property_Size: SetSize(Value.ToPoint()); return true;
	case Property_Rect: SetRect(Value.ToRect()); return true;
	}
	ASSERT(false);
	return false;
}


void cSpriteBase::SetPosition(cPoint Position)
{
	if (!CheckIfChangableProperty(Property_Position))
		return;
	mRect.position() = Position;
	PropertiesSet(Property_Position);
}

void cSpriteBase::SetRect(const cRect& Rect)
{
	if (!CheckIfChangableProperty(Property_Rect))
		return;
	mRect = Rect;
	PropertiesSet(Property_Rect);
}

void cSpriteBase::SetSize(cPoint Size)
{
	if (!CheckIfChangableProperty(Property_Size))
		return;
	mRect.size() = Size;
	PropertiesSet(Property_Size);
}

cPoint cSpriteBase::center() const
{
	return GetRect().center();
}

cPoint cSpriteBase::GetScreenPosition() const
{
	cPixieWindow* Window = GetWindow();
	if (Window)
		return Window->GetScreenRect().position() + GetPosition();
	else
		return GetPosition();
}

void cSpriteBase::SetCenter(cPoint Center)
{
	cPoint Size = GetSize();
	SetPosition({ Center.x - Size.x / 2, Center.y - Size.y / 2 });
}

cRect cSpriteBase::GetCenterAndHSize() const
{
	return { GetRect().center(), GetSize() / 2 };
}

void cSpriteBase::SetCenterAndHSize(const cRect& Rect)
{
	SetRect(cRect::aroundPoint(Rect.position(), Rect.size() * 2));
}

void cSpriteBase::SetScreenPosition(cPoint Position)
{
	cPixieWindow* Window = GetWindow();
	if (!Window)
		SetPosition(Position);
	else
		SetPosition(Position - Window->GetScreenRect().position());
}

cRect cSpriteBase::GetRectForRendering() const
{
	cRect RectForRendering(mRect);
	RectForRendering.left<cRect::PreserveSize>() += mPositionOffset.x;
	RectForRendering.top<cRect::PreserveSize>() += mPositionOffset.y;
	if (mWindow)
	{
		cRect WindowRect = mWindow->GetScreenRect();
		RectForRendering.left<cRect::PreserveSize>() += WindowRect.left();
		RectForRendering.top<cRect::PreserveSize>() += WindowRect.top();
	}
	return RectForRendering;
}

void cSpriteBase::SetPositionOffset(const cPoint& PositionOffset)
{
	if (!CheckIfChangableProperty(Property_PositionOffset))
		return;
	mPositionOffset = PositionOffset;
	PropertiesSet(Property_PositionOffset);
}


int cSpriteBase::mDebugIDCounter = 0;

cSpriteBase::cSpriteBase()
	: mDebugID(++mDebugIDCounter)
{
	mProperties.mColor.SetRGBColor(~0);
}

cSpriteBase::~cSpriteBase()
{
	if (mWindow && IsVisible())
		mWindow->RemoveSprite(this);
}

void cSpriteBase::Drop()
{
	mIsOwned = false;
	if (!mWindow || !IsVisible())
		delete this;
}

void cSpriteBase::Destroy()
{
	delete this;
}

bool cSpriteBase::IsDestroyable() const
{
	if (mIsOwned)
		return false;
	bool CanDestroy = true;
	const_cast<std::remove_const<decltype(mAnimators2)>::type&>(mAnimators2).ForEach([&CanDestroy](auto& Animator) { if (Animator->GetFlags() & cPixieObjectAnimator::KeepsObjectAlive) CanDestroy = false; });
	return CanDestroy;
}

cSpriteBase::eDestroyZombieResult cSpriteBase::DestroyZombie()
{
	if (IsDestroyable())
	{
		mWindow = nullptr; // this is valid ... as long as we are called from cPixieWindow::CheckOwnerlessSprites
		delete this;
		return eDestroyZombieResult::Destroyed;
	}
	return eDestroyZombieResult::StillAlive;
}


void cSpriteBase::SetRotation(float Rotation)
{
	if (!CheckIfChangableProperty(Property_Rotation))
		return;
	// 	if(Rotation>360)
	// 		Rotation%=360;
	// 	else if(Rotation<0)
	// 		Rotation=360+(Rotation%360);
	mProperties.mRotation = Rotation;
	PropertiesSet(Property_Rotation);
}

void cSpriteBase::SetRGBColor(cColor Color)
{
	if (!CheckIfChangableProperty(Property_Color))
		return;
	mProperties.mColor.SetRGBColor(Color.GetRGBColor());
	mIsColorSet = true;
	PropertiesSet(Property_Color);
}

void cSpriteBase::SetARGBColor(uint32_t Color)
{
	if (!CheckIfChangableProperty(Property_Color | Property_Alpha))
		return;
	mProperties.mColor.SetRGBColor(Color);
	mIsColorSet = true;
	PropertiesSet(Property_Color | Property_Alpha);
}

void cSpriteBase::SetAlpha(DWORD Alpha)
{
	if (!CheckIfChangableProperty(Property_Alpha))
		return;
	mProperties.mColor.SetAlpha(Alpha);
	PropertiesSet(Property_Alpha);
}

void cSpriteBase::SetZOrder(int ZOrder)
{
	if (!CheckIfChangableProperty(Property_ZOrder))
		return;
	if (mProperties.mZOrder == ZOrder)
		return;
	if (mWindow && IsVisible())
		mWindow->RemoveSprite(this);
	mProperties.mZOrder = ZOrder;
	if (mWindow && IsVisible())
		mWindow->AddSprite(this);
	PropertiesSet(Property_ZOrder);
}

void cSpriteBase::setShader(std::shared_ptr<cPixelShader> Shader)
{
	if (!CheckIfChangableProperty(Property_Shader))
		return;
	if (mProperties.mShader == Shader)
		return;
	mProperties.mShader = Shader;
	PropertiesChanged(Property_Shader);
}

void cSpriteBase::setShader(const std::string& shaderId)
{
	auto shader = theShaderManager->pixelShader(shaderId);
	if (!shader)
	{
		MainLog->Log("Warning: Shader '{}' not found.", shaderId);
	}
	setShader(std::move(shader));
}

void cSpriteBase::Show()
{
	if (!CheckIfChangableProperty(Property_Visibility))
		return;
	if (mProperties.mVisible)
		return;
	mProperties.mVisible = true;
	if (mWindow)
		mWindow->AddSprite(this);
	PropertiesSet(Property_Visibility);
}

void cSpriteBase::Hide()
{
	if (!CheckIfChangableProperty(Property_Visibility))
		return;
	if (!mProperties.mVisible)
		return;
	mProperties.mVisible = false;
	if (mWindow)
		mWindow->RemoveSprite(this);
	PropertiesSet(Property_Visibility);
}

void cSpriteBase::SetVisible(bool IsVisible)
{
	if (IsVisible)
		Show();
	else
		Hide();
}

void cSpriteBase::SetWindow(cPixieWindow* Window)
{
	if (!CheckIfChangableProperty(Property_Window))
		return;
	if (mWindow && IsVisible())
		mWindow->RemoveSprite(this);
	mWindow = Window;
	if (mWindow && IsVisible())
		mWindow->AddSprite(this);
	PropertiesChanged(Property_Window);
}

void cSpriteBase::setShaderParam(int index, float value)
{
	ASSERT(index >= 0 && index < 4);
	unsigned int flag = Property_ShaderParam0 << index;
	if (!CheckIfChangableProperty(flag))
		return;
	if (mProperties.mShaderParameters[index] == value)
		return;
	mProperties.mShaderParameters[index] = value;
	PropertiesChanged(flag);
}

float cSpriteBase::getShaderParam(int index) const
{
	return mProperties.mShaderParameters[index];
}

int cSpriteBase::shaderParamIndex(std::string_view name) const
{
	if (!mProperties.mShader)
	{
		throw std::runtime_error("No shader set for this renderable.");
	}
	return mProperties.mShader->parameterIndex(name);
}



bool cSpriteBase::GetFloatProperty(unsigned int PropertyFlags, OUT float& Value) const
{
	switch (PropertyFlags)
	{
	case Property_Rotation: Value = GetRotation(); return true;
	}
	ASSERT(false);
	return false;
}

bool cSpriteBase::SetFloatProperty(unsigned int PropertyFlags, float Value)
{
	if (!CheckIfChangableProperty(PropertyFlags))
		return false;
	switch (PropertyFlags)
	{
	case Property_Rotation: SetRotation(Value); return true;
	}
	ASSERT(false);
	return false;
}

bool cSpriteBase::SetStringProperty(unsigned int PropertyFlags, const std::string& Value)
{
	if (!CheckIfChangableProperty(PropertyFlags))
		return false;
	ASSERT(false);
	return false;
}

void cSpriteBase::CopyProperties(const cSpriteBase& source)
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

tIntrusivePtr<cPixieObjectAnimator> blendShaderParam(
	cSpriteBase& sprite,
	int paramOffset,
	float targetValue,
	int blendTime)
{
	std::print("blendShaderParam called with paramOffset: {}, targetValue: {}, blendTime: {}\n", paramOffset, targetValue, blendTime);
	ASSERT(paramOffset >= 0 && paramOffset <= 3);
	return cGeneralPixieObjectBlender::BlendObject(sprite, cPixieObject::cPropertyValues(targetValue), cPixieObject::Property_ShaderParam0 << paramOffset, blendTime);
}

tIntrusivePtr<cPixieObjectAnimator> blendShaderParam(
	cSpriteBase& sprite,
	std::string_view paramID,
	float targetValue,
	int blendTime)
{
	auto index = sprite.shaderParamIndex(paramID);
	if (index < 0)
	{
		ASSERTFALSE("Invalid shader parameter ID");
		return nullptr;
	}
	return cGeneralPixieObjectBlender::BlendObject(sprite, targetValue, cPixieObject::Property_ShaderParam0 << index, blendTime);
}
