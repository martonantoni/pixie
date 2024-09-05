#pragma once

/*

enum class eVerticalAlign { Top, Bottom, Center };
enum class eHorizontalAlign { Left, Right, Center };

template<class T> concept cPoints = requires(T t) 
{ 
    t.begin(); 
    t.end(); 
	t.empty();
    *t.begin(); 
    cPoint(*t.begin()); 
};

struct cRect
{
	int mLeft;
	int mTop;
	int mWidth;
	int mHeight;

	RECT ToRECT() const;

	constexpr cRect(): mLeft(0), mTop(0), mWidth(1), mHeight(1) {}
	constexpr cRect(int Left,int Top,int Width,int Height): mLeft(Left), mTop(Top), mWidth(Width), mHeight(Height) {}
	constexpr cRect(cPoint Position, cPoint Size): mLeft(Position.x), mTop(Position.y), mWidth(Size.x), mHeight(Size.y) {}
	cRect(const cConfig &Config);
	enum class eIsOptional { Yes, No };
	void FromConfig(const cConfig &Config, eIsOptional IsOptional=eIsOptional::No);
	void ToConfig(cConfig &Config) const;
	constexpr int Left() const { return mLeft; }
    constexpr int Top() const { return mTop; }
    constexpr int Right() const { return mLeft+mWidth-1; }
    constexpr int Bottom() const { return mTop+mHeight-1; }
    constexpr int Width() const { return mWidth; }
    constexpr int width() const { return mWidth; }
    constexpr int Height() const { return mHeight; }
	constexpr int height() const { return mHeight; }
	void SetRight(int Right) { mWidth=Right-mLeft+1; }
	void SetBottom(int Bottom) { mHeight=Bottom-mTop+1; }
	void SetPosition(cPoint Position) { mLeft=Position.x; mTop=Position.y; }
	void SetSize(cPoint Size) { mWidth=Size.x; mHeight=Size.y; }
	cPoint GetPosition() const { return cPoint(mLeft,mTop); }
	cPoint position() const { return cPoint(mLeft,mTop); }
	cPoint GetSize() const { return cPoint(mWidth,mHeight); }
	cPoint size() const { return cPoint(mWidth,mHeight); }
	cRect Move(cPoint Offset) { mLeft += Offset.x; mTop += Offset.y; return *this; }
	cRect Grow(cPoint Offset) { mWidth += Offset.x; mHeight += Offset.y; return *this; }
	bool isPointInside(cPoint Point) const;
	bool hasOverlap(const cRect &Other) const;
	bool operator==(const cRect &Other) const { return mLeft==Other.mLeft&&mTop==Other.mTop&&mWidth==Other.mWidth&&mHeight==Other.mHeight; }
	cPoint GetCenter() const { return { mLeft+mWidth/2, mTop+mHeight/2 }; }
	cPoint TopLeft() const { return cPoint(mLeft, mTop); }
	cPoint TopRight() const { return cPoint(Right(), mTop); }
	cPoint BottomLeft() const { return cPoint(mLeft, Bottom()); }
	cPoint BottomRight() const { return cPoint(Right(), Bottom()); }
	cRect GetAlignedRect(const cRect &RectToAlign, eHorizontalAlign HorizontalAlign, eVerticalAlign VerticalAlign) const;
	cRect GetCenteredRect(const cRect &RectToCenter) const { return GetAlignedRect(RectToCenter, eHorizontalAlign::Center, eVerticalAlign::Center); }
	static constexpr cRect createAroundPoint(cPoint point, cPoint size) { return cRect(point - size / 2, size); }
	template<cPoints T> static cRect CreateBoundingBox(const T &Points);
	cRect GetWithModifiedSize(cPoint SizeOffset) const;
	void growToBound(const cRect &RectToBound);
};

inline bool cRect::hasOverlap(const cRect& Other) const
{
	return mLeft < Other.Right() && Right() > Other.mLeft && mTop < Other.Bottom() && Bottom() > Other.mTop;
}

inline bool cRect::isPointInside(cPoint Point) const
{ 
	return Point.x >= mLeft && Point.x < mLeft + mWidth && Point.y >= mTop && Point.y < mTop + mHeight; 
}

template<cPoints T> cRect cRect::CreateBoundingBox(const T& points)
{
	if (points.empty())
		return {};
	cRect rect(points.front(), { 1,1 });
	for (auto& point : points | std::views::drop(1))
	{
		if (point.x < rect.left())
		{
			rect.mWidth += rect.mLeft - point.x;
			rect.mLeft = point.x;
		}
		else if (point.x > rect.right())
		{
			rect.mWidth += point.x - rect.right();
		}
		if (point.y < rect.top())
		{
			rect.mHeight += rect.mTop - point.y;
			rect.mTop = point.y;
		}
		else if (point.y > rect.Bottom())
		{
			rect.mHeight += point.y - rect.Bottom();
		}
	}
	return rect;
}*/