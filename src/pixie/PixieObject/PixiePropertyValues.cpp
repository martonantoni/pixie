#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cPixieObject::cPropertyValues::cPropertyValues(std::initializer_list<int> Initializer)
{
	if(ASSERTFALSE(Initializer.size()>4))
	{
		mPropertyCount=0;
		return;
	}
	mPropertyCount=Initializer.size();
	size_t Index=0;
	for(int v: Initializer)
	{
		mProperties[Index]=v;
		++Index;
	}
}


const cPixieObject::cPropertyValues &cPixieObject::cPropertyValues::operator=(int Value)
{
	mPropertyCount=1;
	mProperties[0]=Value;
	return *this;
}

const cPixieObject::cPropertyValues &cPixieObject::cPropertyValues::operator=(const cPoint &Point)
{
	mPropertyCount=2;
	mProperties[0]=Point.x;
	mProperties[1]=Point.y;
	return *this;
}

const cPixieObject::cPropertyValues &cPixieObject::cPropertyValues::operator=(const cRect &Rect)
{
	mPropertyCount=4;
	mProperties[0]=Rect.mLeft;
	mProperties[1]=Rect.mTop;
	mProperties[2]=Rect.mWidth;
	mProperties[3]=Rect.mHeight;
	return *this;
}

const cPixieObject::cPropertyValues &cPixieObject::cPropertyValues::operator=(const cColor &ColorInfo)
{
	mPropertyCount=3;
	mProperties[0]=ColorInfo.GetRed();
	mProperties[1]=ColorInfo.GetGreen();
	mProperties[2]=ColorInfo.GetBlue();
	return *this;
}

bool cPixieObject::cPropertyValues::operator==(const cPropertyValues &Other) const
{
	if(mPropertyCount!=Other.mPropertyCount)
		return false;
	for(size_t i=0; i!=mPropertyCount; ++i)
	{
		if(mProperties[i]!=Other.mProperties[i])
			return false;
	}
	return true;
}

int cPixieObject::cPropertyValues::ToInt() const
{
	ASSERT(mPropertyCount==1);
	return mProperties[0];
}

cPoint cPixieObject::cPropertyValues::ToPoint() const
{
	ASSERT(mPropertyCount==2);
	return cPoint(mProperties[0], mProperties[1]);
}

cRect cPixieObject::cPropertyValues::ToRect() const
{
	ASSERT(mPropertyCount==4);
	return cRect(mProperties[0], mProperties[1], mProperties[2], mProperties[3]);
}

D3DCOLOR cPixieObject::cPropertyValues::ToRGBColor() const
{
	ASSERT(mPropertyCount==3);
	return D3DCOLOR_XRGB(mProperties[0], mProperties[1], mProperties[2]);
}

