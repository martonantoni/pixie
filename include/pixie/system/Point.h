#pragma once

template<class T>
struct tPoint
{
	T x, y;
	
	constexpr tPoint(): x(0), y(0) {}
	constexpr tPoint(T _x, T _y): x(_x), y(_y) {}
	template<class U> constexpr tPoint(const tPoint<U> &Other): x(Other.x), y(Other.y) {}

	void FromConfig_Point(const cConfig2 &Config);
	void FromConfig_Size(const cConfig2 &Config);
	void FromConfig_Pair(const cConfig2 &Config);
	tPoint &operator-=(const tPoint &Other) { x-=Other.x; y-=Other.y; return *this; }
	tPoint &operator+=(const tPoint &Other) { x+=Other.x; y+=Other.y; return *this; }
	tPoint operator+(const tPoint &Offset) const { return tPoint(x+Offset.x, y+Offset.y); }
	tPoint operator-(const tPoint &Offset) const { return tPoint(x-Offset.x, y-Offset.y); }
	tPoint operator-() const { return tPoint(-x, -y); }
	tPoint operator/(const tPoint &Other) const { return tPoint(x/Other.x, y/Other.y); }
	template<class M> tPoint operator*(M m) const { return tPoint(x*m, y*m); }
	template<class D> tPoint operator/(D d) const { return tPoint(x/d, y/d); }
	template<class U> tPoint &operator=(const tPoint<U> &Other) { x=Other.x;  y=Other.y; }
	bool operator==(const tPoint &Other) const { return x==Other.x&&y==Other.y; }
	bool operator!=(const tPoint &Other) const { return x!=Other.x||y!=Other.y; }
	float DistanceFrom(tPoint Other) const
	{
		int xdist=x-Other.x, ydist=y-Other.y;
		return sqrt(xdist*xdist+ydist*ydist);
	}
	void RatioKeeping_SetX(T nx) { y=y*nx/x; x=nx; }
	void RatioKeeping_SetY(T ny) { x=x*ny/y; y=ny; }
    void Normalize()
    {
        double l = sqrt(x * x + y * y);
        x /= l;
        y /= l;
    }
};

template<class T> inline void tPoint<T>::FromConfig_Point(const cConfig2 &Config)
{
	x=Config.get<T>("x", {});
	y=Config.get<T>("y", {});
}

template<class T> inline void tPoint<T>::FromConfig_Size(const cConfig2 &Config)
{
	x=Config.get<T>("w", {});
	y=Config.get<T>("h", {});
}

template<class T> inline void tPoint<T>::FromConfig_Pair(const cConfig2 &Config)
{
	x=Config.get<T>(0, {});
	y=Config.get<T>(1, {});
}

typedef tPoint<int> cPoint;
typedef tPoint<float> cFloatPoint;

template<class T>
inline cFloatPoint Lerp(T a, T b, float t)
{
	return { a.x+(b.x-a.x)*t, a.y+(b.y-a.y)*t };
}

