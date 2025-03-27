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
	}
    return c2DRenderable::GetProperty(PropertyFlags, PropertyValues);
}

bool cSpriteBase::SetProperty(unsigned int PropertyFlags, const cPropertyValues& Value)
{
	if (!CheckIfChangableProperty(PropertyFlags))
		return false;
	switch (PropertyFlags)
	{
	case Property_X: SetPosition(Value.ToInt(), GetY()); return true;
	case Property_Y: SetPosition(GetX(), Value.ToInt()); return true;
	case Property_W: SetSize(Value.ToInt(), GetHeight()); return true;
	case Property_H: SetSize(GetWidth(), Value.ToInt()); return true;
	case Property_Rotation: SetRotation(Value.ToInt()); return true;
	case Property_Position: SetPosition(Value.ToPoint()); return true;
	case Property_ScreenPosition: SetScreenPosition(Value.ToPoint()); return true;
	case Property_PositionOffset: SetPositionOffset(Value.ToPoint()); return true;
	case Property_CenterAndHSize: SetCenterAndHSize(Value.ToRect()); return true;
	case Property_Center: SetCenter(Value.ToPoint()); return true;
	case Property_Size: SetSize(Value.ToPoint()); return true;
	case Property_Rect: SetRect(Value.ToRect()); return true;
	case Property_ValidRect: SetValidRect(Value.ToRect()); return true;
	}
    return c2DRenderable::SetProperty(PropertyFlags, Value);
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
