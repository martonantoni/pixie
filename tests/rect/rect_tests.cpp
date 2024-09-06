#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"




//////////////////////////////////////////////////////////////////////////
// creation



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
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    testedRect.position() = cPoint(5, 6);  // set
    EXPECT_EQ(testedRect.position(), cPoint(5, 6)); 

    testedRect.position() += cPoint(1, 2);  // get + set
    EXPECT_EQ(testedRect.position(), cPoint(6, 8)); 
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

TEST(Rect, center)
{
    cRect testedRect(10, 20, 30, 40); // left, top, width, height

    EXPECT_EQ(testedRect.center(), cPoint(25, 40)); // get

    testedRect.center() = cPoint(50, 60); // set
    EXPECT_EQ(testedRect.center(), cPoint(50, 60));
    EXPECT_EQ(testedRect.position(), cPoint(35, 40));
    EXPECT_EQ(testedRect.size(), cPoint(30, 40));

    testedRect.center() += cPoint(10, 20); // get + set
    EXPECT_EQ(testedRect.center(), cPoint(60, 80));
    EXPECT_EQ(testedRect.position(), cPoint(45, 60));
    EXPECT_EQ(testedRect.size(), cPoint(30, 40));
}

TEST(Rect, size)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

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
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

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
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

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
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.left(), 1); // get

    testedRect.left() = 2;  // set
    EXPECT_EQ(testedRect.left(), 2);
    EXPECT_EQ(testedRect.position(), cPoint(2, 2));
    EXPECT_EQ(testedRect.size(), cPoint(2, 4));
}

TEST(Rect, left_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.left(), 1); // get

    testedRect.left<cRect::PreserveSize>() = 2;  // set
    EXPECT_EQ(testedRect.left(), 2);
    EXPECT_EQ(testedRect.position(), cPoint(2, 2));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

TEST(Rect, top)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.top(), 2); // get

    testedRect.top() = 3;  // set
    EXPECT_EQ(testedRect.top(), 3);
    EXPECT_EQ(testedRect.position(), cPoint(1, 3));
    EXPECT_EQ(testedRect.size(), cPoint(3, 3));
}

TEST(Rect, top_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.top(), 2); // get

    testedRect.top<cRect::PreserveSize>() = 3;  // set
    EXPECT_EQ(testedRect.top(), 3);
    EXPECT_EQ(testedRect.position(), cPoint(1, 3));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

TEST(Rect, bottom)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.bottom(), 6); // get

    testedRect.bottom() = 7;  // set
    EXPECT_EQ(testedRect.bottom(), 7);
    EXPECT_EQ(testedRect.position(), cPoint(1, 2));
    EXPECT_EQ(testedRect.size(), cPoint(3, 5));
}

TEST(Rect, bottom_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.bottom(), 6); // get

    testedRect.bottom<cRect::PreserveSize>() = 7;  // set
    EXPECT_EQ(testedRect.bottom(), 7);
    EXPECT_EQ(testedRect.position(), cPoint(1, 3));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

TEST(Rect, right)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.right(), 4); // get

    testedRect.right() = 5;  // set
    EXPECT_EQ(testedRect.right(), 5);
    EXPECT_EQ(testedRect.position(), cPoint(1, 2));
    EXPECT_EQ(testedRect.size(), cPoint(4, 4));
}

TEST(Rect, right_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.right(), 4); // 1 + 3 = 4

    testedRect.right<cRect::PreserveSize>() = 5;  // move --> +1
    EXPECT_EQ(testedRect.right(), 5);
    EXPECT_EQ(testedRect.position(), cPoint(2, 2));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}


TEST(Rect, bottomRight)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

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
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.bottomRight(), cPoint(4, 6)); // get

    testedRect.bottomRight<cRect::PreserveSize>() = cPoint(5, 7);  // set
    EXPECT_EQ(testedRect.bottomRight(), cPoint(5, 7));
    EXPECT_EQ(testedRect.position(), cPoint(2, 3)); // check that position is changed
    EXPECT_EQ(testedRect.size(), cPoint(3, 4)); // check that size is not changed

    testedRect.bottomRight<cRect::PreserveSize>() += cPoint(1, 2);  // get + set
    EXPECT_EQ(testedRect.bottomRight(), cPoint(6, 9));
    EXPECT_EQ(testedRect.position(), cPoint(3, 5)); // check that position is changed
    EXPECT_EQ(testedRect.size(), cPoint(3, 4)); // check that size is not changed
}

TEST(Rect, bottomLeft)  // topRight stays in place
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

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
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.bottomLeft(), cPoint(1, 6)); // get

    testedRect.bottomLeft<cRect::PreserveSize>() = cPoint(2, 7);  // set
    EXPECT_EQ(testedRect.bottomLeft(), cPoint(2, 7));
    EXPECT_EQ(testedRect.position(), cPoint(2, 7 - 4));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

TEST(Rect, topRight)  // bottomLeft stays in place
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.topRight(), cPoint(4, 2)); // get

    testedRect.topRight() = cPoint(5, 3);  // set
    EXPECT_EQ(testedRect.topRight(), cPoint(5, 3));
    EXPECT_EQ(testedRect.position(), cPoint(1, 3));
    EXPECT_EQ(testedRect.size(), cPoint(4, 3));
    EXPECT_EQ(testedRect.bottomLeft(), cPoint(1, 6));
}

TEST(Rect, topRight_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.topRight(), cPoint(4, 2)); // get

    testedRect.topRight<cRect::PreserveSize>() = cPoint(5, 3);  // set
    EXPECT_EQ(testedRect.topRight(), cPoint(5, 3));
    EXPECT_EQ(testedRect.position(), cPoint(5 - 3, 3));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}

TEST(Rect, topLeft)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.topLeft(), cPoint(1, 2)); // get

    testedRect.topLeft() = cPoint(2, 3);  // basically it's a move with { 1, 1 }
    EXPECT_EQ(testedRect.topLeft(), cPoint(2, 3));
    EXPECT_EQ(testedRect.position(), cPoint(2, 3));
    EXPECT_EQ(testedRect.size(), cPoint(2, 3));
    EXPECT_EQ(testedRect.bottomRight(), cPoint(4, 6)); // did not change
}

TEST(Rect, topLeft_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.topLeft(), cPoint(1, 2)); // get

    testedRect.topLeft<cRect::PreserveSize>() = cPoint(2, 3); // move with { 1, 1 }
    EXPECT_EQ(testedRect.topLeft(), cPoint(2, 3));
    EXPECT_EQ(testedRect.position(), cPoint(2, 3));
    EXPECT_EQ(testedRect.size(), cPoint(3, 4));
}


TEST(Rect, aroundPoint)
{
// odd size
    cRect testedRect = cRect::aroundPoint({ 0, 0 }, { 3, 5 });
    EXPECT_EQ(testedRect.size(), cPoint( 3, 5 ));
    EXPECT_EQ(testedRect.position(), cPoint( -1, -2 ));
// even size: the larger part of the size is on the right and bottom
    testedRect = cRect::aroundPoint({ 0, 0 }, { 4, 6 });
    EXPECT_EQ(testedRect.size(), cPoint( 4, 6 ));
    EXPECT_EQ(testedRect.position(), cPoint( -2, -3 ));
}

void alignTest4x4(eHorizontalAlign horizontal, eVerticalAlign vertical, cPoint expectedPosition)
{
    cRect baseRect(0, 0, 10, 10);
    cRect rectToAlign(0, 0, 4, 4);
    cRect testedRect = cRect::alignedRect(baseRect, rectToAlign, horizontal, vertical);
    EXPECT_EQ(testedRect.position(), expectedPosition);
    EXPECT_EQ(testedRect.size(), cPoint(4, 4));
}

TEST(Rect, alignedRect_TopLeft)
{
    alignTest4x4(eHorizontalAlign::Left, eVerticalAlign::Top, cPoint(0, 0));
}

TEST(Rect, alignedRect_TopCenter)
{
    alignTest4x4(eHorizontalAlign::Center, eVerticalAlign::Top, cPoint(3, 0));
}

TEST(Rect, alignedRect_TopRight)
{
    alignTest4x4(eHorizontalAlign::Right, eVerticalAlign::Top, cPoint(6, 0));
}

TEST(Rect, alignedRect_CenterLeft)
{
    alignTest4x4(eHorizontalAlign::Left, eVerticalAlign::Center, cPoint(0, 3));
}

TEST(Rect, alignedRect_Center)
{
    alignTest4x4(eHorizontalAlign::Center, eVerticalAlign::Center, cPoint(3, 3));
}

TEST(Rect, alignedRect_CenterRight)
{
    alignTest4x4(eHorizontalAlign::Right, eVerticalAlign::Center, cPoint(6, 3));
}

TEST(Rect, alignedRect_BottomLeft)
{
    alignTest4x4(eHorizontalAlign::Left, eVerticalAlign::Bottom, cPoint(0, 6));
}

TEST(Rect, alignedRect_BottomCenter)
{
    alignTest4x4(eHorizontalAlign::Center, eVerticalAlign::Bottom, cPoint(3, 6));
}

TEST(Rect, alignedRect_BottomRight)
{
    alignTest4x4(eHorizontalAlign::Right, eVerticalAlign::Bottom, cPoint(6, 6));
}

TEST(Rect, growToBound_point)
{
    cRect testedRect(0, 0, 3, 3); // left, top, width, height

    testedRect.growToBound(cPoint(0, 1));  // inside: no change
    EXPECT_EQ(testedRect.position(), cPoint(0, 0));
    EXPECT_EQ(testedRect.size(), cPoint(3, 3));

    testedRect.growToBound(cPoint(0, 0));  // inside: no change
    EXPECT_EQ(testedRect.position(), cPoint(0, 0));
    EXPECT_EQ(testedRect.size(), cPoint(3, 3));

    testedRect.growToBound(cPoint(-5, 0));  // outside: grow to the left
    EXPECT_EQ(testedRect.position(), cPoint(-5, 0));
    EXPECT_EQ(testedRect.size(), cPoint(8, 3));

    testedRect.growToBound(cPoint(10, 15));  // outside: grow to the right and bottom
    EXPECT_EQ(testedRect.position(), cPoint(-5, 0));
    EXPECT_EQ(testedRect.size(), cPoint(15, 15));
}

TEST(Rect, growToBound_rect)
{
    cRect testedRect(0, 0, 3, 3); // left, top, width, height

    testedRect.growToBound(cRect(0, 1, 1, 1));  // inside: no change
    EXPECT_EQ(testedRect.position(), cPoint(0, 0));
    EXPECT_EQ(testedRect.size(), cPoint(3, 3));

    testedRect.growToBound(cRect(0, 0, 15, 10)); // outside: grow to the right and bottom
    EXPECT_EQ(testedRect.position(), cPoint(0, 0));
    EXPECT_EQ(testedRect.size(), cPoint(15, 10));

    testedRect.growToBound(cRect(-5, -5, 1, 1)); // outside: grow to the left and top
    EXPECT_EQ(testedRect.position(), cPoint(-5, -5));
    EXPECT_EQ(testedRect.size(), cPoint(20, 15));
}

TEST(Rect, isPointInside)
{
    cRect testedRect(0, 0, 3, 3); // left, top, width, height

    EXPECT_TRUE(testedRect.isPointInside(cPoint(0, 0)));
    EXPECT_TRUE(testedRect.isPointInside(cPoint(2, 2)));
    EXPECT_TRUE(testedRect.isPointInside(cPoint(2, 0)));
    EXPECT_TRUE(testedRect.isPointInside(cPoint(0, 2)));
    EXPECT_TRUE(testedRect.isPointInside(cPoint(2, 2)));

    EXPECT_FALSE(testedRect.isPointInside(cPoint(-1, 0)));
    EXPECT_FALSE(testedRect.isPointInside(cPoint(0, -1)));
    EXPECT_FALSE(testedRect.isPointInside(cPoint(3, 0)));
    EXPECT_FALSE(testedRect.isPointInside(cPoint(0, 3)));
    EXPECT_FALSE(testedRect.isPointInside(cPoint(3, 3)));
}

TEST(Rect, hasOverlap)
{
    cRect testedRect(0, 0, 3, 3); // left, top, width, height

    EXPECT_TRUE(testedRect.hasOverlap(cRect(0, 0, 3, 3))); // same
    EXPECT_TRUE(testedRect.hasOverlap(cRect(0, 0, 1, 1))); // inside
    EXPECT_TRUE(testedRect.hasOverlap(cRect(0, 0, 3, 1))); // inside
    EXPECT_TRUE(testedRect.hasOverlap(cRect(0, 0, 1, 3))); // inside
    EXPECT_TRUE(testedRect.hasOverlap(cRect(0, 0, 4, 4))); // outside
    EXPECT_TRUE(testedRect.hasOverlap(cRect(-1, -1, 10, 10))); // outside

    EXPECT_FALSE(testedRect.hasOverlap(cRect(4, 4, 1, 1))); // outside
    EXPECT_FALSE(testedRect.hasOverlap(cRect(4, 0, 1, 1))); // outside
    EXPECT_FALSE(testedRect.hasOverlap(cRect(0, 4, 1, 1))); // outside
    EXPECT_FALSE(testedRect.hasOverlap(cRect(4, 4, 1, 1))); // outside
    EXPECT_FALSE(testedRect.hasOverlap(cRect(-4, -4, 1, 1))); // outside
    EXPECT_FALSE(testedRect.hasOverlap(cRect(-4, -4, 1000, 3))); // outside
    EXPECT_FALSE(testedRect.hasOverlap(cRect(-4, -4, 3, 1000))); // outside

}


} // namespace RectTests