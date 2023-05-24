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
	auto Default=IsOptional==eIsOptional::Yes?tDefaultValue<int>(0):tDefaultValue<int>();
	mLeft=Config.GetInt("x", Default);
	mTop=Config.GetInt("y", Default);
	mWidth=Config.GetInt("w", Default);
	mHeight=Config.GetInt("h", Default);
}

void cRect::ToConfig(cConfig &Config) const
{
	Config.Set("x"s, mLeft);
	Config.Set("y"s, mTop);
	Config.Set("w"s, mWidth);
	Config.Set("h"s, mHeight);
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
	case eHorizontalAlign::Left:
		Result.mTop=mLeft;
		break;
	case eHorizontalAlign::Right:
		Result.mTop=Bottom()-RectToAlign.mHeight;
		break;
	case eHorizontalAlign::Center:
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

cRect cRect::CreateBoundingBox(const std::vector<cPoint> &Points)
{
	if(Points.empty())
		return {};
	cRect Rect(Points.front(), { 1,1 });
	for(auto &Point: Points)
	{
		if(Point.x<Rect.Left())
		{
			Rect.mWidth+=Rect.mLeft-Point.x;
			Rect.mLeft=Point.x;
		}
		else if(Point.x>Rect.Right())
		{
			Rect.mWidth+=Point.x-Rect.Right();
		}
		if(Point.y<Rect.Top())
		{
			Rect.mHeight+=Rect.mTop-Point.y;
			Rect.mTop=Point.y;
		}
		else if(Point.y>Rect.Bottom())
		{
			Rect.mHeight+=Point.y-Rect.Bottom();
		}
	}
	return Rect;
}
