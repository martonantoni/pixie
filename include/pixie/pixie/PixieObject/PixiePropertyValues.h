#pragma once

struct cPixieObject::cPropertyValues
{
	size_t mPropertyCount;
	int mProperties[4];

	cPropertyValues(): mPropertyCount(0) {}
	cPropertyValues(int Value): mPropertyCount(1) { mProperties[0]=Value; }
	cPropertyValues(const cPoint &Point): mPropertyCount(2) { mProperties[0]=Point.x; mProperties[1]=Point.y; }
	cPropertyValues(const cRect &Rect): mPropertyCount(4) { mProperties[0]=Rect.left(); mProperties[1]=Rect.top(); mProperties[2]=Rect.width(); mProperties[3]=Rect.height(); }
	cPropertyValues(const cColor &ColorInfo): mPropertyCount(3) { mProperties[0]=ColorInfo.GetRed(); mProperties[1]=ColorInfo.GetGreen(); mProperties[2]=ColorInfo.GetBlue(); }
	cPropertyValues(std::initializer_list<int> Initializer);
	const cPropertyValues &operator=(int Value);
	const cPropertyValues &operator=(const cPoint &Point);
	const cPropertyValues &operator=(const cRect &Rect);
	const cPropertyValues &operator=(const cColor &ColorInfo);
	bool operator==(const cPropertyValues &Other) const;
	int ToInt() const;
	cPoint ToPoint() const;
	cRect ToRect() const;
	D3DCOLOR ToRGBColor() const;
};
