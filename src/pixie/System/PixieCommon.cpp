#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
RECT cRect::ToRECT() const
{
	RECT Rect={ mLeft, mTop, Right(), Bottom() };
	return Rect;
}

cRect::cRect(const cConfig &Config)
{
	FromConfig(Config);
}

void cRect::FromConfig(const cConfig &Config, eIsOptional IsOptional)
{
	if (IsOptional == eIsOptional::Yes)
	{
		mLeft = Config.get<int>("x", mLeft);
		mTop = Config.get<int>("y", mTop);
		mWidth = Config.get<int>("w", mWidth);
		mHeight = Config.get<int>("h", mHeight);
	}
	else
	{
		mLeft = Config.get<int>("x");
        mTop = Config.get<int>("y");
        mWidth = Config.get<int>("w");
        mHeight = Config.get<int>("h");
    }
}

void cRect::ToConfig(cConfig &Config) const
{
	Config.set("x"s, mLeft);
	Config.set("y"s, mTop);
	Config.set("w"s, mWidth);
	Config.set("h"s, mHeight);
}

cRect cRect::GetAlignedRect(const cRect &RectToAlign, eHorizontalAlign HorizontalAlign, eVerticalAlign VerticalAlign) const
{
	cRect Result;
	Result.SetSize(RectToAlign.GetSize());
	switch(HorizontalAlign)
	{
	case eHorizontalAlign::Left:
		Result.mLeft=mLeft;
		break;
	case eHorizontalAlign::Right:
		Result.mLeft=Right()-RectToAlign.mWidth;
		break;
	case eHorizontalAlign::Center:
		Result.mLeft=mLeft+(mWidth-RectToAlign.mWidth)/2;
		break;
	}
	switch(VerticalAlign)
	{
	case eVerticalAlign::Top:
		Result.mTop=mLeft;
		break;
	case eVerticalAlign::Bottom:
		Result.mTop=Bottom()-RectToAlign.mHeight;
		break;
	case eVerticalAlign::Center:
		Result.mTop=mTop+(mHeight-RectToAlign.mHeight)/2;
		break;
	}
	return Result;
}

cRect cRect::GetWithModifiedSize(cPoint SizeOffset) const
{
	return FromCenterAndSize(GetCenter(), GetSize()+SizeOffset);
}

void cRect::GrowToBound(const cRect &RectToBound)
{
	if(mLeft>RectToBound.mLeft)
	{
		mWidth+=mLeft-RectToBound.mLeft;
		mLeft=RectToBound.mLeft;
	}
	if(mTop>RectToBound.mTop)
	{
		mHeight+=mTop-RectToBound.mTop;
		mTop=RectToBound.mTop;
	}
	if(Bottom()<RectToBound.Bottom())
		SetBottom(RectToBound.Bottom());
	if(Right()<RectToBound.Right())
		SetRight(RectToBound.Right());
}

cRect cRect::createAroundPoint(cPoint point, cPoint size)
{
	return cRect(point - size / 2, size);
}
