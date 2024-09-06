#include "StdAfx.h"

cRect cRect::aroundPoint(cPoint point, cPoint size)
{
    return cRect(point - size / 2, size);
}

cRect cRect::centeredRect(const cRect& baseRect, const cRect& rectToCenter)
{
    return alignedRect(baseRect, rectToCenter, eHorizontalAlign::Center, eVerticalAlign::Center);
}

void cRect::growToBound(const cPoint& point)
{
	if (point.x < left())
	{
		left() = point.x;
	}
    else if (point.x > right())
    {
        right() = point.x;
    }
	if (point.y < top())
    {
        top() = point.y;
    }
    else if (point.y > bottom())
    {
        bottom() = point.y;
    }
}

void cRect::growToBound(const cRect& RectToBound)
{
    growToBound(RectToBound.topLeft());
    growToBound(RectToBound.bottomRight());
}

cRect cRect::alignedRect(const cRect& baseRect, const cRect& rectToAlign, eHorizontalAlign HorizontalAlign, eVerticalAlign VerticalAlign)
{
	cRect result;
	result.size() = rectToAlign.size();
	switch (HorizontalAlign)
	{
	case eHorizontalAlign::Left:
		result.left<PreserveSize>() = baseRect.left();
		break;
	case eHorizontalAlign::Right:
		result.left<PreserveSize>() = baseRect.right() - rectToAlign.width() + 1;
		break;
	case eHorizontalAlign::Center:
		result.left<PreserveSize>() = baseRect.left() + (baseRect.width() - rectToAlign.width()) / 2;
		break;
	}
	switch (VerticalAlign)
	{
	case eVerticalAlign::Top:
		result.top<PreserveSize>() = baseRect.top();
		break;
	case eVerticalAlign::Bottom:
		result.top<PreserveSize>() = baseRect.bottom() - rectToAlign.height() + 1;
		break;
	case eVerticalAlign::Center:
		result.top<PreserveSize>() = baseRect.top() + (baseRect.height() - rectToAlign.height()) / 2;
		break;
	}
	return result;
}

void cRect::fromConfig(const cConfig& config, eIsOptional isOptional)
{
	if (isOptional == eIsOptional::Yes)
	{
		mTopLeft = { config.get<int>("x", left()), config.get<int>("y", top()) };
		mSize = { config.get<int>("w", width()), config.get<int>("h", height()) };
	}
	else
	{
		mTopLeft = { config.get<int>("x"), config.get<int>("y") };
		mSize = { config.get<int>("w"), config.get<int>("h") };
	}
}

void cRect::toConfig(cConfig& config) const
{
	config.set("x"s, left());
	config.set("y"s, top());
	config.set("w"s, width());
	config.set("h"s, height());
}
