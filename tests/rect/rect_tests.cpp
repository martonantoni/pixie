#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

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

class cRect2;

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
    bool operator==(VALUE_TYPE value) const { return getter(mOwner) == value; }
    bool operator!=(VALUE_TYPE value) const { return getter(mOwner) != value; }
};

class cRect2 final
{
    cPoint mTopLeft;
    cPoint mSize;
public:
    constexpr cRect2() : mTopLeft(0, 0), mSize(0, 0) {}
    constexpr cRect2(int left, int top, int width, int height) : mTopLeft(left, top), mSize(width, height) {}
    constexpr cRect2(cPoint topLeft, cPoint size) : mTopLeft(topLeft), mSize(size) {}

    cPoint position() const { return mTopLeft; }
    cPoint& position() { return mTopLeft; }
    cPoint size() const { return mSize; }
    cPoint& size() { return mSize; }

    cPoint topLeft() const { return mTopLeft; }
    cPoint bottomRight() const { return mTopLeft + mSize; }
    cPoint topRight() const { return mTopLeft + cPoint(mSize.x, 0); }
    cPoint bottomLeft() const { return mTopLeft + cPoint(0, mSize.y); }
    cPoint centerPosition() const { return mTopLeft + mSize / 2; }
    int width() const { return mSize.x; }
    int height() const { return mSize.y; }
    int left() const { return mTopLeft.x; }
    int top() const { return mTopLeft.y; }
    int right() const { return mTopLeft.x + mSize.x; }
    int bottom() const { return mTopLeft.y + mSize.y; }

    auto centerPosition();
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

    void growToBound(const cRect2& rectToBound);
    void growToBound(const cPoint& point);

    static cRect2 aroundPoint(cPoint point, cPoint size);
    static cRect2 alignedRect(const cRect2& baseRect, const cRect2& rectToAlign, eHorizontalAlign horizontalAlign, eVerticalAlign verticalAlign);
    static cRect2 centeredRect(const cRect2& baseRect, const cRect2& rectToCenter);
    template<cPoints T> static cRect2 boundingBox(const T& points);
};


auto cRect2::centerPosition()
{
    return tProxy < cRect2, cPoint,
        [](auto& rect) { return rect.mTopLeft + rect.mSize / 2; },
        [](auto& rect, auto value) { rect.mTopLeft = value - rect.mSize / 2; } > (*this);
}

auto cRect2::width()
{
    return tProxy < cRect2, int,
        [](auto& rect) { return rect.mSize.x; },
        [](auto& rect, auto value) { rect.mSize.x = value; } > (*this);
}

auto cRect2::height()
{
    return tProxy < cRect2, int,
        [](auto& rect) { return rect.mSize.y; },
        [](auto& rect, auto value) { rect.mSize.y = value; } > (*this);
}


template<class... Properties> auto cRect2::bottomRight()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect2, cPoint,
            [](auto& rect) { return rect.mTopLeft + rect.mSize; },
            [](auto& rect, auto value) { rect.mTopLeft = value - rect.mSize; }> (*this);
    }
    else
    {
        return tProxy<cRect2, cPoint,
            [](auto& rect) { return rect.mTopLeft + rect.mSize; },
            [](auto& rect, auto value) { rect.mSize = value - rect.mTopLeft; }> (*this);
    }
}

template<class... Properties> auto cRect2::bottomLeft()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect2, cPoint,
            [](auto& rect) { return rect.mTopLeft + cPoint(0, rect.mSize.y); },
            [](auto& rect, auto value) { rect.mTopLeft = value - cPoint(0, rect.mSize.y); }> (*this);
    }
    else
    {
        return tProxy<cRect2, cPoint,
            [](auto& rect) { return rect.mTopLeft + cPoint(0, rect.mSize.y); },
            [](auto& rect, auto value) // top right corner is fixed
            { 
                rect.mSize.x += rect.mTopLeft.x - value.x; 
                rect.mTopLeft.x = value.x;
                rect.mSize.y = value.y - rect.mTopLeft.y;
            }> (*this);
    }
}

template<class... Properties> auto cRect2::topRight()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect2, cPoint,
            [](auto& rect) { return rect.mTopLeft + cPoint(rect.mSize.x, 0); },
            [](auto& rect, auto value) { rect.mTopLeft = value - cPoint(rect.mSize.x, 0); }> (*this);
    }
    else
    {
        return tProxy<cRect2, cPoint,
            [](auto& rect) { return rect.mTopLeft + cPoint(rect.mSize.x, 0); },
            [](auto& rect, auto value) // bottom left corner is fixed
            {
                rect.mSize.x = value.x - rect.mTopLeft.x;
                rect.mSize.y += rect.mTopLeft.y - value.y;
                rect.mTopLeft.y = value.y;
            }> (*this);
    }
}

template<class... Properties> auto cRect2::topLeft()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect2, cPoint,
            [](auto& rect) { return rect.mTopLeft; },
            [](auto& rect, auto value) { rect.mTopLeft = value; }> (*this);
    }
    else
    {
        return tProxy<cRect2, cPoint,
            [](auto& rect) { return rect.mTopLeft; },
            [](auto& rect, auto value) { rect.mSize += rect.mTopLeft - value; rect.mTopLeft = value; }> (*this);
    }
}


template<class... Properties> auto cRect2::left()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect2, int,
            [](auto& rect) { return rect.mTopLeft.x; },
            [](auto& rect, auto value) { rect.mTopLeft.x = value; }> (*this);
    }
    else
    {
        return tProxy<cRect2, int,
            [](auto& rect) { return rect.mTopLeft.x; },
            [](auto& rect, auto value) { rect.mSize.x += rect.mTopLeft.x - value; rect.mTopLeft.x = value; }> (*this);
    }
}

template<class... Properties> auto cRect2::top()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect2, int,
            [](auto& rect) { return rect.mTopLeft.y; },
            [](auto& rect, auto value) { rect.mTopLeft.y = value; }> (*this);
    }
    else
    {
        return tProxy<cRect2, int,
            [](auto& rect) { return rect.mTopLeft.y; },
            [](auto& rect, auto value) { rect.mSize.y += rect.mTopLeft.y - value; rect.mTopLeft.y = value; }> (*this);
    }
}

template<class... Properties> auto cRect2::right()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect2, int,
            [](auto& rect) { return rect.mTopLeft.x + rect.mSize.x; },
            [](auto& rect, auto value) { rect.mTopLeft.x = value - rect.mSize.x; }> (*this);
    }
    else
    {
        return tProxy<cRect2, int,
            [](auto& rect) { return rect.mTopLeft.x + rect.mSize.x; },
            [](auto& rect, auto value) { rect.mSize.x = value - rect.mTopLeft.x; }> (*this);
    }
}

template<class... Properties> auto cRect2::bottom()
{
    if constexpr (std::disjunction_v<std::is_same<Properties, PreserveSize>...>)
    {
        return tProxy<cRect2, int,
            [](auto& rect) { return rect.mTopLeft.y + rect.mSize.y; },
            [](auto& rect, auto value) { rect.mTopLeft.y = value - rect.mSize.y; }> (*this);
    }
    else
    {
        return tProxy < cRect2, int,
            [](auto& rect) { return rect.mTopLeft.y + rect.mSize.y; },
            [](auto& rect, auto value) { rect.mSize.y = value - rect.mTopLeft.y; } > (*this);
    }
}

void cRect2::fromConfig(const cConfig& config, eIsOptional isOptional)
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

void cRect2::toConfig(cConfig& config) const
{
    config.set("x"s, left());
    config.set("y"s, top());
    config.set("w"s, width());
    config.set("h"s, height());
}


//////////////////////////////////////////////////////////////////////////
// creation


cRect2 cRect2::alignedRect(const cRect2& baseRect, const cRect2& RectToAlign, eHorizontalAlign HorizontalAlign, eVerticalAlign VerticalAlign)
{
    return {};
}

cRect2 cRect2::aroundPoint(cPoint point, cPoint size)
{ 
    return cRect2(point - size / 2, size); 
}


cRect2 cRect2::centeredRect(const cRect2& baseRect, const cRect2& rectToCenter)
{
    return alignedRect(baseRect, rectToCenter, eHorizontalAlign::Center, eVerticalAlign::Center);
}

template<cPoints T> cRect2 cRect2::boundingBox(const T & points)
{
    return {};
}


inline std::ostream& operator<<(std::ostream& os, const cPoint& point)
{
    os << "(" << point.x << ", " << point.y << ")";
    return os;
}

namespace ProxyTests
{

TEST(Proxy, value)
{
    int testedValue = 50;
    auto testedProxy = tProxy<int, int,
        [](auto& value) { return value; },
        [](auto& value, auto newValue) { value = newValue; } > (testedValue);

    EXPECT_EQ(testedValue, 50);
    EXPECT_EQ(testedProxy, 50);
}

TEST(Proxy, assignment)
{
    int testedValue = 50;
    auto testedProxy = tProxy<int, int,
        [](auto& value) { return value; },
        [](auto& value, auto newValue) { value = newValue; } > (testedValue);

    EXPECT_EQ(testedValue, 50);
    testedProxy = 60;
    EXPECT_EQ(testedValue, 60);
    testedProxy = 70;
    EXPECT_EQ(testedValue, 70);
}

TEST(Proxy, compound_assignment)
{
    std::pair<int, int> testedValue(50, 60);
    auto testedProxy = tProxy < std::pair<int, int>, int,
        [](auto& value) { return value.first; },
        [](auto& value, auto newValue) { value.first = newValue; } > (testedValue);

    EXPECT_EQ(testedValue.first, 50);
    testedProxy += 10;
    EXPECT_EQ(testedValue.first, 60);
    testedProxy -= 20;
    EXPECT_EQ(testedValue.first, 40);
    testedProxy *= 2;
    EXPECT_EQ(testedValue.first, 80);
    testedProxy /= 4;
    EXPECT_EQ(testedValue.first, 20);
}

TEST(Proxy, comparison)
{
    int testedValue = 50;
    auto testedProxy = tProxy<int, int,
        [](auto& value) { return value; },
        [](auto& value, auto newValue) { value = newValue; } > (testedValue);

    EXPECT_EQ(testedProxy, 50);
    EXPECT_EQ(testedProxy, 50);
    EXPECT_NE(testedProxy, 60);
    EXPECT_NE(testedProxy, 40);
}

} // namespace ProxyTests

namespace RectTests
{

TEST(Rect, position)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    testedRect.position() = cPoint(5, 6);  // set
    EXPECT_EQ(testedRect.position(), cPoint(5, 6)); 

    testedRect.position() += cPoint(1, 2);  // get + set
    EXPECT_EQ(testedRect.position(), cPoint(6, 8)); 
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

TEST(Rect, centerPosition)
{
    cRect2 testedRect(10, 20, 30, 40); // left, top, width, height

    EXPECT_EQ(testedRect.centerPosition(), cPoint(25, 40)); // get

    testedRect.centerPosition() = cPoint(50, 60); // set
    EXPECT_EQ(testedRect.centerPosition(), cPoint(50, 60));
    EXPECT_EQ(testedRect.position(), cPoint(35, 40));
    EXPECT_EQ(testedRect.size(), cPoint(30, 40));

    testedRect.centerPosition() += cPoint(10, 20); // get + set
    EXPECT_EQ(testedRect.centerPosition(), cPoint(60, 80));
    EXPECT_EQ(testedRect.position(), cPoint(45, 60));
    EXPECT_EQ(testedRect.size(), cPoint(30, 40));
}

TEST(Rect, size)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.size(), cPoint(3, 4)); // get

    testedRect.size() = cPoint(5, 6);  // set
    EXPECT_EQ(testedRect.size(), cPoint(5, 6));
    EXPECT_EQ(testedRect.position(), cPoint(1, 2)); // check that position is not changed

    testedRect.size() += cPoint(1, 2);  // get + set
    EXPECT_EQ(testedRect.size(), cPoint(6, 8));
    EXPECT_EQ(testedRect.position(), cPoint(1, 2)); // check that position is not changed
}

TEST(Rect, width)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.width(), 3); // get

    testedRect.width() = 5;  // set
    EXPECT_EQ(testedRect.width(), 5);
    EXPECT_EQ(testedRect.size(), cPoint(5, 4)); // check that only width is changed
    EXPECT_EQ(testedRect.position(), cPoint(1, 2)); // check that position is not changed

    testedRect.width() += 1;  // get + set
    EXPECT_EQ(testedRect.width(), 6);
    EXPECT_EQ(testedRect.size(), cPoint(6, 4)); // check that size is changed (but only width)
    EXPECT_EQ(testedRect.position(), cPoint(1, 2)); // check that position is not changed
}

TEST(Rect, height)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.height(), 4); // get

    testedRect.height() = 5;  // set
    EXPECT_EQ(testedRect.height(), 5);
    EXPECT_EQ(testedRect.size(), cPoint(3, 5)); // check that only height is changed
    EXPECT_EQ(testedRect.position(), cPoint(1, 2)); // check that position is not changed

    testedRect.height() += 1;  // get + set
    EXPECT_EQ(testedRect.height(), 6);
    EXPECT_EQ(testedRect.size(), cPoint(3, 6)); // check that size is changed (but only height)
    EXPECT_EQ(testedRect.position(), cPoint(1, 2)); // check that position is not changed
}


TEST(Rect, left)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.left(), 1); // get

    testedRect.left() = 2;  // set
    EXPECT_EQ(testedRect.left(), 2);
    EXPECT_EQ(testedRect.position(), cPoint(2, 2));
    EXPECT_EQ(testedRect.size(), cPoint(2, 4));
}

TEST(Rect, left_preserveSize)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.left(), 1); // get

    testedRect.left<cRect2::PreserveSize>() = 2;  // set
    EXPECT_EQ(testedRect.left(), 2);
    EXPECT_EQ(testedRect.position(), cPoint(2, 2));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

TEST(Rect, top)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.top(), 2); // get

    testedRect.top() = 3;  // set
    EXPECT_EQ(testedRect.top(), 3);
    EXPECT_EQ(testedRect.position(), cPoint(1, 3));
    EXPECT_EQ(testedRect.size(), cPoint(3, 3));
}

TEST(Rect, top_preserveSize)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.top(), 2); // get

    testedRect.top<cRect2::PreserveSize>() = 3;  // set
    EXPECT_EQ(testedRect.top(), 3);
    EXPECT_EQ(testedRect.position(), cPoint(1, 3));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

TEST(Rect, bottom)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.bottom(), 6); // get

    testedRect.bottom() = 7;  // set
    EXPECT_EQ(testedRect.bottom(), 7);
    EXPECT_EQ(testedRect.position(), cPoint(1, 2));
    EXPECT_EQ(testedRect.size(), cPoint(3, 5));
}

TEST(Rect, bottom_preserveSize)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.bottom(), 6); // get

    testedRect.bottom<cRect2::PreserveSize>() = 7;  // set
    EXPECT_EQ(testedRect.bottom(), 7);
    EXPECT_EQ(testedRect.position(), cPoint(1, 3));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

TEST(Rect, right)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.right(), 4); // get

    testedRect.right() = 5;  // set
    EXPECT_EQ(testedRect.right(), 5);
    EXPECT_EQ(testedRect.position(), cPoint(1, 2));
    EXPECT_EQ(testedRect.size(), cPoint(4, 4));
}

TEST(Rect, right_preserveSize)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.right(), 4); // 1 + 3 = 4

    testedRect.right<cRect2::PreserveSize>() = 5;  // move --> +1
    EXPECT_EQ(testedRect.right(), 5);
    EXPECT_EQ(testedRect.position(), cPoint(2, 2));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}


TEST(Rect, bottomRight)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.bottomRight(), cPoint(4, 6)); // get

    testedRect.bottomRight() = cPoint(5, 7);  // set
    EXPECT_EQ(testedRect.bottomRight(), cPoint(5, 7));
    EXPECT_EQ(testedRect.position(), cPoint(1, 2)); // must not change position
    EXPECT_EQ(testedRect.size(), cPoint(4, 5));

    testedRect.bottomRight() += cPoint(1, 2);  // get + set
    EXPECT_EQ(testedRect.bottomRight(), cPoint(6, 9));
    EXPECT_EQ(testedRect.position(), cPoint(1, 2)); // must not change position
    EXPECT_EQ(testedRect.size(), cPoint(5, 7)); // check that only width and height are changed

}

TEST(Rect, bottomRight_preserveSize)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.bottomRight(), cPoint(4, 6)); // get

    testedRect.bottomRight<cRect2::PreserveSize>() = cPoint(5, 7);  // set
    EXPECT_EQ(testedRect.bottomRight(), cPoint(5, 7));
    EXPECT_EQ(testedRect.position(), cPoint(2, 3)); // check that position is changed
    EXPECT_EQ(testedRect.size(), cPoint(3, 4)); // check that size is not changed

    testedRect.bottomRight<cRect2::PreserveSize>() += cPoint(1, 2);  // get + set
    EXPECT_EQ(testedRect.bottomRight(), cPoint(6, 9));
    EXPECT_EQ(testedRect.position(), cPoint(3, 5)); // check that position is changed
    EXPECT_EQ(testedRect.size(), cPoint(3, 4)); // check that size is not changed
}

TEST(Rect, bottomLeft)  // topRight stays in place
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.bottomLeft(), cPoint(1, 6)); // get

    testedRect.bottomLeft() = cPoint(2, 7);  // set
    EXPECT_EQ(testedRect.bottomLeft(), cPoint(2, 7));
    EXPECT_EQ(testedRect.position(), cPoint(2, 2));
    EXPECT_EQ(testedRect.size(), cPoint(2, 5));
    EXPECT_EQ(testedRect.topRight(), cPoint(4, 2));

    testedRect.bottomLeft() += cPoint(1, 2);  // get + set
    EXPECT_EQ(testedRect.bottomLeft(), cPoint(3, 9));
    EXPECT_EQ(testedRect.position(), cPoint(3, 2));
    EXPECT_EQ(testedRect.size(), cPoint(1, 7));
}

TEST(Rect, bottomLeft_preserveSize)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.bottomLeft(), cPoint(1, 6)); // get

    testedRect.bottomLeft<cRect2::PreserveSize>() = cPoint(2, 7);  // set
    EXPECT_EQ(testedRect.bottomLeft(), cPoint(2, 7));
    EXPECT_EQ(testedRect.position(), cPoint(2, 7 - 4));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

TEST(Rect, topRight)  // bottomLeft stays in place
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.topRight(), cPoint(4, 2)); // get

    testedRect.topRight() = cPoint(5, 3);  // set
    EXPECT_EQ(testedRect.topRight(), cPoint(5, 3));
    EXPECT_EQ(testedRect.position(), cPoint(1, 3));
    EXPECT_EQ(testedRect.size(), cPoint(4, 3));
    EXPECT_EQ(testedRect.bottomLeft(), cPoint(1, 6));
}

TEST(Rect, topRight_preserveSize)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.topRight(), cPoint(4, 2)); // get

    testedRect.topRight<cRect2::PreserveSize>() = cPoint(5, 3);  // set
    EXPECT_EQ(testedRect.topRight(), cPoint(5, 3));
    EXPECT_EQ(testedRect.position(), cPoint(5 - 3, 3));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

TEST(Rect, topLeft)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.topLeft(), cPoint(1, 2)); // get

    testedRect.topLeft() = cPoint(2, 3);  // basically it's a move with { 1, 1 }
    EXPECT_EQ(testedRect.topLeft(), cPoint(2, 3));
    EXPECT_EQ(testedRect.position(), cPoint(2, 3));
    EXPECT_EQ(testedRect.size(), cPoint(2, 3));
    EXPECT_EQ(testedRect.bottomRight(), cPoint(4, 6)); // did not change
}

TEST(Rect, topLeft_preserveSize)
{
    cRect2 testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.topLeft(), cPoint(1, 2)); // get

    testedRect.topLeft<cRect2::PreserveSize>() = cPoint(2, 3); // move with { 1, 1 }
    EXPECT_EQ(testedRect.topLeft(), cPoint(2, 3));
    EXPECT_EQ(testedRect.position(), cPoint(2, 3));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

} // namespace RectTests