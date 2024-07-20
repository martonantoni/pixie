#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

class cRect2;

template<class OWNER, class VALUE_TYPE, auto getter, auto setter> class tProxy
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

class cRect2
{
    cPoint mTopLeft;
    cPoint mSize;
public:
    constexpr cRect2() : mTopLeft(0, 0), mSize(0, 0) {}
    constexpr cRect2(int left, int top, int width, int height) : mTopLeft(left, top), mSize(width, height) {}
    constexpr cRect2(cPoint topLeft, cPoint size) : mTopLeft(topLeft), mSize(size) {}

    cPoint topLeft() const { return mTopLeft; }
    cPoint& topLeft() { return mTopLeft; }
    cPoint position() const { return mTopLeft; }
    cPoint& position() { return mTopLeft; }
    cPoint size() const { return mSize; }
    cPoint& size() { return mSize; }

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

    auto bottomRight();
    auto bottomLeft();
    auto centerPosition();
    auto width();
    auto height();
    auto left();
    auto top();
    auto right();
    auto bottom();
};

auto cRect2::bottomRight()
{
    return tProxy<cRect2, cPoint,
        [](auto& rect) { return rect.mTopLeft + rect.mSize; },
        [](auto& rect, auto value) { rect.mSize = value - rect.mTopLeft; }> (*this);
}

auto cRect2::bottomLeft()
{
    return tProxy<cRect2, cPoint,
        [](auto& rect) { return rect.mTopLeft + cPoint(0, rect.mSize.y); },
        [](auto& rect, auto value) { rect.mSize.y = value.y - rect.mTopLeft.y; rect.mTopLeft.x = value.x; }> (*this);
}

auto cRect2::centerPosition()
{
    return tProxy<cRect2, cPoint,
        [](auto& rect) { return rect.mTopLeft + rect.mSize / 2; },
        [](auto& rect, auto value) { rect.mTopLeft = value - rect.mSize / 2; }> (*this);
}

auto cRect2::width()
{
    return tProxy<cRect2, int,
        [](auto& rect) { return rect.mSize.x; },
        [](auto& rect, auto value) { rect.mSize.x = value; }> (*this);
}

auto cRect2::height()
{
    return tProxy<cRect2, int,
        [](auto& rect) { return rect.mSize.y; },
        [](auto& rect, auto value) { rect.mSize.y = value; }> (*this);
}

auto cRect2::left()
{
    return tProxy<cRect2, int,
        [](auto& rect) { return rect.mTopLeft.x; },
        [](auto& rect, auto value) { rect.mSize.x += rect.mTopLeft.x - value; rect.mTopLeft.x = value; }> (*this);
}

auto cRect2::top()
{
    return tProxy<cRect2, int,
        [](auto& rect) { return rect.mTopLeft.y; },
        [](auto& rect, auto value) { rect.mSize.y += rect.mTopLeft.y - value; rect.mTopLeft.y = value; }> (*this);
}

auto cRect2::right()
{
    return tProxy<cRect2, int,
        [](auto& rect) { return rect.mTopLeft.x + rect.mSize.x; },
        [](auto& rect, auto value) { rect.mSize.x = value - rect.mTopLeft.x; }> (*this);
}

auto cRect2::bottom()
{
    return tProxy<cRect2, int,
        [](auto& rect) { return rect.mTopLeft.y + rect.mSize.y; },
        [](auto& rect, auto value) { rect.mSize.y = value - rect.mTopLeft.y; }> (*this);
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
    cRect2 testedRect(1, 2, 3, 4); // top, left, width, height

    EXPECT_EQ(testedRect.position(), cPoint(1, 2));
    testedRect.position() = cPoint(5, 6);
    EXPECT_EQ(testedRect.position(), cPoint(5, 6));
    testedRect.position() += cPoint(1, 2);
    EXPECT_EQ(testedRect.position(), cPoint(6, 8));
    testedRect.position() -= cPoint(2, 1);
    EXPECT_EQ(testedRect.position(), cPoint(4, 7));
    
    // check if size did not get modified:
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

TEST(Rect, centerPosition)
{
    cRect2 testedRect(10, 20, 30, 40); // top, left, width, height

    EXPECT_EQ(testedRect.centerPosition(), cPoint(25, 40));
    testedRect.centerPosition() = cPoint(50, 60);
    EXPECT_EQ(testedRect.centerPosition(), cPoint(50, 60));
    EXPECT_EQ(testedRect.position(), cPoint(35, 40));
    EXPECT_EQ(testedRect.size(), cPoint(30, 40));

    testedRect.centerPosition() += cPoint(10, 20);
    EXPECT_EQ(testedRect.centerPosition(), cPoint(60, 80));
    EXPECT_EQ(testedRect.position(), cPoint(45, 60));
    EXPECT_EQ(testedRect.size(), cPoint(30, 40));

    testedRect.centerPosition() -= cPoint(20, 10);
    EXPECT_EQ(testedRect.centerPosition(), cPoint(40, 70));
    EXPECT_EQ(testedRect.position(), cPoint(25, 50));
    EXPECT_EQ(testedRect.size(), cPoint(30, 40));
}

TEST(Rect, size)
{
    cRect2 testedRect(1, 2, 3, 4); // top, left, width, height

    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
    testedRect.size() = cPoint(5, 6);
    EXPECT_EQ(testedRect.size(), cPoint(5, 6));
    testedRect.size() += cPoint(1, 2);
    EXPECT_EQ(testedRect.size(), cPoint(6, 8));
    testedRect.size() -= cPoint(2, 1);
    EXPECT_EQ(testedRect.size(), cPoint(4, 7));
    
    // check if position did not get modified:
    EXPECT_EQ(testedRect.position(), cPoint(1, 2));
}

} // namespace RectTests