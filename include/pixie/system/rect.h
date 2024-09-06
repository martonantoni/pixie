#pragma once

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

class cRect;

template<class OWNER, class VALUE_TYPE, auto getter, auto setter> class tProxy final
{
    OWNER& mOwner;
public:
    tProxy(OWNER& owner) : mOwner(owner) {}
    tProxy(const tProxy&) = delete;
    tProxy(tProxy&&) = delete;
    tProxy& operator=(const tProxy&) = delete;
    tProxy& operator=(tProxy&&) = delete;
    operator VALUE_TYPE() const { return getter(mOwner); }
    tProxy& operator=(VALUE_TYPE value) { setter(mOwner, value); return *this; }
    tProxy& operator+=(VALUE_TYPE value) { setter(mOwner, getter(mOwner) + value); return *this; }
    tProxy& operator-=(VALUE_TYPE value) { setter(mOwner, getter(mOwner) - value); return *this; }
    tProxy& operator*=(VALUE_TYPE value) { setter(mOwner, getter(mOwner) * value); return *this; }
    tProxy& operator/=(VALUE_TYPE value) { setter(mOwner, getter(mOwner) / value); return *this; }

    template<class T>
        requires std::convertible_to<decltype(std::declval<VALUE_TYPE>() + std::declval<T>()), VALUE_TYPE>
    VALUE_TYPE operator+(T value) const { return getter(mOwner) + value; }

    template<class T>
        requires std::convertible_to<decltype(std::declval<VALUE_TYPE>() - std::declval<T>()), VALUE_TYPE>
    VALUE_TYPE operator-(T value) const { return getter(mOwner) - value; }

    template<class T> 
        requires std::convertible_to<decltype(std::declval<VALUE_TYPE>() * std::declval<T>()), VALUE_TYPE>
    VALUE_TYPE operator*(T value) const { return getter(mOwner) * value; }

    template<class T>
        requires std::convertible_to<decltype(std::declval<VALUE_TYPE>() / std::declval<T>()), VALUE_TYPE>
    VALUE_TYPE operator/(T value) const { return getter(mOwner) / value; }

    bool operator==(VALUE_TYPE value) const { return getter(mOwner) == value; }
    bool operator!=(VALUE_TYPE value) const { return getter(mOwner) != value; }
    template<std::constructible_from<VALUE_TYPE> T> operator T() const { return getter(mOwner); }
};

class cRect final
{
    cPoint mTopLeft;
    cPoint mSize;
public:
    constexpr cRect() : mTopLeft(0, 0), mSize(0, 0) {}
    constexpr cRect(int left, int top, int width, int height) : mTopLeft(left, top), mSize(width, height) {}
    constexpr cRect(cPoint position, cPoint size) : mTopLeft(position), mSize(size) {}
    cRect(const cConfig& config) { fromConfig(config); }

    cPoint position() const { return mTopLeft; }
    cPoint& position() { return mTopLeft; }
    cPoint size() const { return mSize; }
    cPoint& size() { return mSize; }
    cRect& grow(cPoint offset) { mSize += offset; return *this; }
    cRect& move(cPoint offset) { mTopLeft += offset; return *this; }

    cPoint topLeft() const { return mTopLeft; }
    cPoint bottomRight() const { return mTopLeft + mSize + cPoint { -1, -1 }; }
    cPoint topRight() const { return mTopLeft + cPoint(mSize.x - 1, 0); }
    cPoint bottomLeft() const { return mTopLeft + cPoint(0, mSize.y - 1); }
    cPoint center() const { return mTopLeft + mSize / 2; }
    int width() const { return mSize.x; }
    int height() const { return mSize.y; }
    int left() const { return mTopLeft.x; }
    int top() const { return mTopLeft.y; }
    int right() const { return mTopLeft.x + mSize.x - 1; }
    int bottom() const { return mTopLeft.y + mSize.y - 1; }

    auto center();
    auto width();
    auto height();

    struct PreserveSize {};
    // corners:
    template<class... Properties> auto bottomRight();
    template<class... Properties> auto bottomLeft();
    template<class... Properties> auto topRight();
    template<class... Properties> auto topLeft();
    // sides:
    template<class... Properties> auto left();
    template<class... Properties> auto top();
    template<class... Properties> auto right();
    template<class... Properties> auto bottom();

    enum class eIsOptional { Yes, No };
    void fromConfig(const cConfig& config, eIsOptional isOptional = eIsOptional::No);
    void toConfig(cConfig& config) const;

    void growToBound(const cRect& rectToBound);
    void growToBound(const cPoint& point);

    static cRect aroundPoint(cPoint point, cPoint size);
    static cRect alignedRect(const cRect& baseRect, const cRect& rectToAlign, eHorizontalAlign horizontalAlign, eVerticalAlign verticalAlign);
    static cRect centeredRect(const cRect& baseRect, const cRect& rectToCenter);
    template<cPoints T> static cRect boundingBox(const T& points);

    bool isPointInside(cPoint point) const;
    bool hasOverlap(const cRect& other) const;

    bool operator==(const cRect& other) const { return mTopLeft == other.mTopLeft && mSize == other.mSize; }
    bool operator!=(const cRect& other) const { return mTopLeft != other.mTopLeft || mSize != other.mSize; }
};


inline auto cRect::center()
{
    return tProxy<cRect, cPoint,
        [](auto& rect) { return rect.mTopLeft + rect.mSize / 2; },
        [](auto& rect, auto value) { rect.mTopLeft = value - rect.mSize / 2; }> (*this);
}

inline auto cRect::width()
{
    return tProxy<cRect, int,
        [](auto& rect) { return rect.mSize.x; },
        [](auto& rect, auto value) { rect.mSize.x = value; }> (*this);
}

inline auto cRect::height()
{
    return tProxy<cRect, int,
        [](auto& rect) { return rect.mSize.y; },
        [](auto& rect, auto value) { rect.mSize.y = value; }> (*this);
}


template<class... Properties> auto cRect::bottomRight()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect, cPoint,
            [](auto& rect) { return rect.mTopLeft + rect.mSize - cPoint{ 1, 1 }; },
            [](auto& rect, auto value) { rect.mTopLeft = value - rect.mSize + cPoint{ 1, 1 }; }> (*this);
    }
    else
    {
        return tProxy < cRect, cPoint,
            [](auto& rect) { return rect.mTopLeft + rect.mSize - cPoint{ 1, 1 }; },
            [](auto& rect, auto value) { rect.mSize = value - rect.mTopLeft + cPoint{ 1, 1 }; }> (*this);
    }
}

template<class... Properties> auto cRect::bottomLeft()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect, cPoint,
            [](auto& rect) { return rect.mTopLeft + cPoint(0, rect.mSize.y - 1); },
            [](auto& rect, auto value) { rect.mTopLeft = value - cPoint(0, rect.mSize.y - 1); }> (*this);
    }
    else
    {
        return tProxy<cRect, cPoint,
            [](auto& rect) { return rect.mTopLeft + cPoint(0, rect.mSize.y - 1); },
            [](auto& rect, auto value) // top right corner is fixed
            {
                rect.mSize.x += rect.mTopLeft.x - value.x;
                rect.mSize.y = value.y - rect.mTopLeft.y + 1;
                rect.mTopLeft.x = value.x;
            }> (*this);
    }
}

template<class... Properties> auto cRect::topRight()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect, cPoint,
            [](auto& rect) { return rect.mTopLeft + cPoint(rect.mSize.x - 1, 0); },
            [](auto& rect, auto value) { rect.mTopLeft = value - cPoint(rect.mSize.x - 1, 0); }> (*this);
    }
    else
    {
        return tProxy<cRect, cPoint,
            [](auto& rect) { return rect.mTopLeft + cPoint(rect.mSize.x - 1, 0); },
            [](auto& rect, auto value) // bottom left corner is fixed
            {
                rect.mSize.x = value.x - rect.mTopLeft.x + 1;
                rect.mSize.y += rect.mTopLeft.y - value.y;
                rect.mTopLeft.y = value.y;
            }> (*this);
    }
}

template<class... Properties> auto cRect::topLeft()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect, cPoint,
            [](auto& rect) { return rect.mTopLeft; },
            [](auto& rect, auto value) { rect.mTopLeft = value; }> (*this);
    }
    else
    {
        return tProxy<cRect, cPoint,
            [](auto& rect) { return rect.mTopLeft; },
            [](auto& rect, auto value) { rect.mSize += rect.mTopLeft - value; rect.mTopLeft = value; }> (*this);
    }
}


template<class... Properties> auto cRect::left()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect, int,
            [](auto& rect) { return rect.mTopLeft.x; },
            [](auto& rect, auto value) { rect.mTopLeft.x = value; }> (*this);
    }
    else
    {
        return tProxy<cRect, int,
            [](auto& rect) { return rect.mTopLeft.x; },
            [](auto& rect, auto value) { rect.mSize.x += rect.mTopLeft.x - value; rect.mTopLeft.x = value; }> (*this);
    }
}

template<class... Properties> auto cRect::top()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect, int,
            [](auto& rect) { return rect.mTopLeft.y; },
            [](auto& rect, auto value) { rect.mTopLeft.y = value; }> (*this);
    }
    else
    {
        return tProxy<cRect, int,
            [](auto& rect) { return rect.mTopLeft.y; },
            [](auto& rect, auto value) { rect.mSize.y += rect.mTopLeft.y - value; rect.mTopLeft.y = value; }> (*this);
    }
}

template<class... Properties> auto cRect::right()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect, int,
            [](auto& rect) { return rect.mTopLeft.x + rect.mSize.x - 1; },
            [](auto& rect, auto value) { rect.mTopLeft.x = value - rect.mSize.x + 1; }> (*this);
    }
    else
    {
        return tProxy<cRect, int,
            [](auto& rect) { return rect.mTopLeft.x + rect.mSize.x - 1; },
            [](auto& rect, auto value) { rect.mSize.x = value - rect.mTopLeft.x + 1; }> (*this);
    }
}

template<class... Properties> auto cRect::bottom()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect, int,
            [](auto& rect) { return rect.mTopLeft.y + rect.mSize.y - 1; },
            [](auto& rect, auto value) { rect.mTopLeft.y = value - rect.mSize.y + 1; }> (*this);
    }
    else
    {
        return tProxy<cRect, int,
            [](auto& rect) { return rect.mTopLeft.y + rect.mSize.y - 1; },
            [](auto& rect, auto value) { rect.mSize.y = value - rect.mTopLeft.y + 1; }> (*this);
    }
}

inline bool cRect::isPointInside(cPoint point) const
{
    return point.x >= left() && point.x <= right() && point.y >= top() && point.y <= bottom();
}

inline bool cRect::hasOverlap(const cRect& other) const
{
    return left() <= other.right() && right() >= other.left() && top() <= other.bottom() && bottom() >= other.top();
}

template<cPoints T> cRect cRect::boundingBox(const T& points)
{
    cRect rect(points.front(), { 1, 1 });
    for (auto& point : points | std::views::drop(1))
    {
        rect.growToBound(point);
    }
    return rect;
}
