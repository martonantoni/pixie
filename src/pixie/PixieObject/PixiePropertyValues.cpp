#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
//cPixieObject::cPropertyValues::cPropertyValues(std::initializer_list<int> Initializer)
//{
//	if(ASSERTFALSE(Initializer.size()>4))
//	{
//		mPropertyCount=0;
//		return;
//	}
//	mPropertyCount=Initializer.size();
//	size_t Index=0;
//	for(int v: Initializer)
//	{
//		mProperties[Index]=v;
//		++Index;
//	}
//}


//const cPixieObject::cPropertyValues &cPixieObject::cPropertyValues::operator=(int Value)
//{
//	mPropertyCount=1;
//	mProperties[0]=Value;
//	return *this;
//}
//
//const cPixieObject::cPropertyValues &cPixieObject::cPropertyValues::operator=(const cPoint &Point)
//{
//	mPropertyCount=2;
//	mProperties[0]=Point.x;
//	mProperties[1]=Point.y;
//	return *this;
//}
//
//const cPixieObject::cPropertyValues &cPixieObject::cPropertyValues::operator=(const cRect &Rect)
//{
//	mPropertyCount=4;
//	mProperties[0]=Rect.left();
//	mProperties[1]=Rect.top();
//	mProperties[2]=Rect.width();
//	mProperties[3]=Rect.height();
//	return *this;
//}
//
//const cPixieObject::cPropertyValues &cPixieObject::cPropertyValues::operator=(const cColor &ColorInfo)
//{
//	mPropertyCount=3;
//	mProperties[0]=ColorInfo.GetRed();
//	mProperties[1]=ColorInfo.GetGreen();
//	mProperties[2]=ColorInfo.GetBlue();
//	return *this;
//}
//
//bool cPixieObject::cPropertyValues::operator==(const cPropertyValues &Other) const
//{
//	if(mPropertyCount!=Other.mPropertyCount)
//		return false;
//	for(size_t i=0; i!=mPropertyCount; ++i)
//	{
//		if(mProperties[i]!=Other.mProperties[i])
//			return false;
//	}
//	return true;
//}
//
//int cPixieObject::cPropertyValues::ToInt() const
//{
//	ASSERT(mPropertyCount==1);
//	return mProperties[0];
//}
//
//cPoint cPixieObject::cPropertyValues::ToPoint() const
//{
//	ASSERT(mPropertyCount==2);
//	return cPoint(mProperties[0], mProperties[1]);
//}
//
//cRect cPixieObject::cPropertyValues::ToRect() const
//{
//	ASSERT(mPropertyCount==4);
//	return cRect(mProperties[0], mProperties[1], mProperties[2], mProperties[3]);
//}
//
//uint32_t cPixieObject::cPropertyValues::ToRGBColor() const
//{
//	ASSERT(mPropertyCount == 3);
//
//	return 0xFF000000 |
//		(static_cast<uint32_t>(mProperties[0]) << 16) |
//		(static_cast<uint32_t>(mProperties[1]) << 8) |
//		static_cast<uint32_t>(mProperties[2]);
//}

cPixieObject::cPropertyValues cPixieObject::cPropertyValues::lerp(const cPropertyValues& a, const cPropertyValues& b, float t)
{
    if (a.mValue.index() != b.mValue.index())
        throw std::invalid_argument("Cannot interpolate between different types of property values.");
    if (std::holds_alternative<int>(a.mValue))
    {
        int valueA = std::get<int>(a.mValue);
        int valueB = std::get<int>(b.mValue);
        return cPropertyValues(static_cast<int>(valueA + (valueB - valueA) * t));
    }
    else if (std::holds_alternative<float>(a.mValue))
    {
        float valueA = std::get<float>(a.mValue);
        float valueB = std::get<float>(b.mValue);
        return cPropertyValues(valueA + (valueB - valueA) * t);
    }
    else if (std::holds_alternative<cPoint>(a.mValue))
    {
        cPoint pointA = std::get<cPoint>(a.mValue);
        cPoint pointB = std::get<cPoint>(b.mValue);
        return cPropertyValues(cPoint(
            static_cast<int>(pointA.x + (pointB.x - pointA.x) * t),
            static_cast<int>(pointA.y + (pointB.y - pointA.y) * t)
        ));
    }
    else if (std::holds_alternative<cRect>(a.mValue))
    {
        cRect rectA = std::get<cRect>(a.mValue);
        cRect rectB = std::get<cRect>(b.mValue);
        return cPropertyValues(cRect(
            static_cast<int>(rectA.left() + (rectB.left() - rectA.left()) * t),
            static_cast<int>(rectA.top() + (rectB.top() - rectA.top()) * t),
            static_cast<int>(rectA.width() + (rectB.width() - rectA.width()) * t),
            static_cast<int>(rectA.height() + (rectB.height() - rectA.height()) * t)
        ));
    }
    else if (std::holds_alternative<cColor>(a.mValue))
    {
        cColor colorA = std::get<cColor>(a.mValue);
        cColor colorB = std::get<cColor>(b.mValue);
        return cPropertyValues(cColor(
            static_cast<int>(colorA.GetRed() + (colorB.GetRed() - colorA.GetRed()) * t),
            static_cast<int>(colorA.GetGreen() + (colorB.GetGreen() - colorA.GetGreen()) * t),
            static_cast<int>(colorA.GetBlue() + (colorB.GetBlue() - colorA.GetBlue()) * t)
        ));
    }

    throw std::invalid_argument("Unsupported property value type for interpolation.");
}
