#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"


inline std::ostream& operator<<(std::ostream& os, const cPoint& point)
{
    os << "(" << point.x << ", " << point.y << ")";
    return os;
}

namespace RectTests
{

struct cExpectedRectCoords
{
    cPoint topLeft;
    cPoint topRight;
    cPoint bottomLeft;
    cPoint bottomRight;
};

struct cMovedOriginal
{
    cPoint offset;
    operator cExpectedRectCoords() const
    {
        return
        {
            cPoint(1 + offset.x, 2 + offset.y),
            cPoint(3 + offset.x, 2 + offset.y),
            cPoint(1 + offset.x, 5 + offset.y),
            cPoint(3 + offset.x, 5 + offset.y)
        };
    }
};

void testRectChange(auto set, const cExpectedRectCoords& expected)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    set(testedRect);  // set

    EXPECT_EQ(testedRect.topLeft(), expected.topLeft);
    EXPECT_EQ(testedRect.topRight(), expected.topRight);
    EXPECT_EQ(testedRect.bottomLeft(), expected.bottomLeft);
    EXPECT_EQ(testedRect.bottomRight(), expected.bottomRight);
    EXPECT_EQ(testedRect.position(), expected.topLeft);
    EXPECT_EQ(testedRect.size(), expected.bottomRight - expected.topLeft + cPoint(1, 1));
    EXPECT_EQ(testedRect.width(), expected.bottomRight.x - expected.topLeft.x + 1);
    EXPECT_EQ(testedRect.height(), expected.bottomRight.y - expected.topLeft.y + 1);
    EXPECT_EQ(testedRect.top(), expected.topLeft.y);
    EXPECT_EQ(testedRect.left(), expected.topLeft.x);
    EXPECT_EQ(testedRect.bottom(), expected.bottomLeft.y);
    EXPECT_EQ(testedRect.right(), expected.topRight.x);
    EXPECT_EQ(const_cast<const cRect&>(testedRect).topLeft(), expected.topLeft);
    EXPECT_EQ(const_cast<const cRect&>(testedRect).topRight(), expected.topRight);
    EXPECT_EQ(const_cast<const cRect&>(testedRect).bottomLeft(), expected.bottomLeft);
    EXPECT_EQ(const_cast<const cRect&>(testedRect).bottomRight(), expected.bottomRight);
    EXPECT_EQ(const_cast<const cRect&>(testedRect).position(), expected.topLeft);
    EXPECT_EQ(const_cast<const cRect&>(testedRect).size(), expected.bottomRight - expected.topLeft + cPoint(1, 1));
    EXPECT_EQ(const_cast<const cRect&>(testedRect).width(), expected.bottomRight.x - expected.topLeft.x + 1);
    EXPECT_EQ(const_cast<const cRect&>(testedRect).height(), expected.bottomRight.y - expected.topLeft.y + 1);
    EXPECT_EQ(const_cast<const cRect&>(testedRect).top(), expected.topLeft.y);
    EXPECT_EQ(const_cast<const cRect&>(testedRect).left(), expected.topLeft.x);
    EXPECT_EQ(const_cast<const cRect&>(testedRect).bottom(), expected.bottomLeft.y);
    EXPECT_EQ(const_cast<const cRect&>(testedRect).right(), expected.topRight.x);
}

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

    testRectChange(
        [](cRect& rect) { rect.size() = cPoint(5, 7); },
        { 
            cPoint(1, 2), cPoint(5, 2), 
            cPoint(1, 8), cPoint(5, 8) 
        });
}

TEST(Rect, width)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.width(), 3); // get
    EXPECT_EQ(const_cast<cRect&>(testedRect).width(), 3); // get

    testRectChange(
        [](cRect& rect) { rect.width() = 5; },
        { 
            cPoint(1, 2), cPoint(5, 2), 
            cPoint(1, 5), cPoint(5, 5) 
        });
}

TEST(Rect, height)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.height(), 4); // get
    EXPECT_EQ(const_cast<cRect&>(testedRect).height(), 4); // get

    testRectChange(
        [](cRect& rect) { rect.height() = 5; },
        { 
            cPoint(1, 2), cPoint(3, 2), 
            cPoint(1, 6), cPoint(3, 6) 
        });
}

TEST(Rect, left)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height

    EXPECT_EQ(testedRect.left(), 1); // get
    EXPECT_EQ(const_cast<cRect&>(testedRect).left(), 1); // get

    testRectChange(
        [](cRect& rect) { rect.left() = 2; },
        { 
            cPoint(2, 2), cPoint(3, 2), 
            cPoint(2, 5), cPoint(3, 5) 
        });
}

TEST(Rect, left_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.left(), 1); // get

    testRectChange(
        [](cRect& rect) { rect.left<cRect::PreserveSize>() = 2; },
        // this is effectively a move with { 1, 0 }
        cMovedOriginal({ 1, 0 }));
}

TEST(Rect, top)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.top(), 2); // get
    EXPECT_EQ(const_cast<cRect&>(testedRect).top(), 2); // get

    testRectChange(
        [](cRect& rect) { rect.top() = 3; },
        { 
            cPoint(1, 3), cPoint(3, 3), 
            cPoint(1, 5), cPoint(3, 5) 
        });
}

TEST(Rect, top_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.top<cRect::PreserveSize>(), 2); // get

    testRectChange(
        [](cRect& rect) { rect.top<cRect::PreserveSize>() = 3; },
        // this is effectively a move with { 0, 1 }
        cMovedOriginal({ 0, 1 }));
}

TEST(Rect, bottom)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.bottom(), 5); // get
    EXPECT_EQ(const_cast<cRect&>(testedRect).bottom(), 5); // get

    testRectChange(
        [](cRect& rect) { rect.bottom() = 7; },
        { 
            cPoint(1, 2), cPoint(3, 2), 
            cPoint(1, 7), cPoint(3, 7) 
        });
}

TEST(Rect, bottom_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.bottom<cRect::PreserveSize>(), 5); // get

    testRectChange(
        [](cRect& rect) { rect.bottom<cRect::PreserveSize>() = 7; },
        // this is effectively a move with { 0, 2 }
        cMovedOriginal({ 0, 2 }));
}

TEST(Rect, right)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.right(), 3); // get
    EXPECT_EQ(const_cast<cRect&>(testedRect).right(), 3); // get

    testRectChange(
        [](cRect& rect) { rect.right() = 5; },
        { 
            cPoint(1, 2), cPoint(5, 2), 
            cPoint(1, 5), cPoint(5, 5) 
        });
}

TEST(Rect, right_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.right<cRect::PreserveSize>(), 3); // 1 + 3 = 4

    testRectChange(
        [](cRect& rect) { rect.right<cRect::PreserveSize>() = 5; },
        // this is effectively a move with { 2, 0 }
        cMovedOriginal({ 2, 0 }));
}

TEST(Rect, bottomRight)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.bottomRight(), cPoint(3, 5)); // get
    EXPECT_EQ(const_cast<const cRect&>(testedRect).bottomRight(), cPoint(3, 5)); // get

    testRectChange(
        [](cRect& rect) { rect.bottomRight() = cPoint(5, 7); },
        { 
            cPoint(1, 2), cPoint(5, 2), 
            cPoint(1, 7), cPoint(5, 7) 
        });
}

TEST(Rect, bottomRight_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.bottomRight<cRect::PreserveSize>(), cPoint(3, 5)); // get

    testRectChange(
        [](cRect& rect) { rect.bottomRight<cRect::PreserveSize>() = cPoint(5, 8); }, 
        // this is effectively a move with { 2, 3 }
        cMovedOriginal({ 2, 3 }));
}

TEST(Rect, bottomLeft)  // topRight stays in place
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.bottomLeft(), cPoint(1, 5)); // get
    EXPECT_EQ(const_cast<const cRect&>(testedRect).bottomLeft(), cPoint(1, 5)); // get

    testRectChange(
        [](cRect& rect) { rect.bottomLeft() = cPoint(2, 7); },
        { 
            cPoint(2, 2), cPoint(3, 2), 
            cPoint(2, 7), cPoint(3, 7) 
        });
}

TEST(Rect, bottomLeft_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.bottomLeft<cRect::PreserveSize>(), cPoint(1, 5)); // get

    testRectChange(
        [](cRect& rect) { rect.bottomLeft<cRect::PreserveSize>() = cPoint(2, 7); },
        // this is effectively a move with { 1, 2 }
        cMovedOriginal({ 1, 2 }));
}

TEST(Rect, topRight)  // bottomLeft stays in place
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.topRight(), cPoint(3, 2)); // get
    EXPECT_EQ(const_cast<const cRect&>(testedRect).topRight(), cPoint(3, 2)); // get

    testRectChange(
        [](cRect& rect) { rect.topRight() = cPoint(5, 3); },
        { 
            cPoint(1, 3), cPoint(5, 3), 
            cPoint(1, 5), cPoint(5, 5) 
        });
}

TEST(Rect, topRight_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.topRight<cRect::PreserveSize>(), cPoint(3, 2)); // get

    testRectChange(
        [](cRect& rect) { rect.topRight<cRect::PreserveSize>() = cPoint(5, 3); },
        // this is effectively a move with { 2, 1 }
        cMovedOriginal({ 2, 1 }));
}

TEST(Rect, topLeft)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.topLeft(), cPoint(1, 2)); // get
    EXPECT_EQ(const_cast<const cRect&>(testedRect).topLeft(), cPoint(1, 2)); // get

    testRectChange(
        [](cRect& rect) { rect.topLeft() = cPoint(2, 3); },
        { 
            cPoint(2, 3), cPoint(3, 3), 
            cPoint(2, 5), cPoint(3, 5) 
        });
}

TEST(Rect, topLeft_preserveSize)
{
    cRect testedRect(1, 2, 3, 4); // left, top, width, height
    EXPECT_EQ(testedRect.topLeft<cRect::PreserveSize>(), cPoint(1, 2)); // get

    testRectChange(
        [](cRect& rect) { rect.topLeft<cRect::PreserveSize>() = cPoint(3, 4); },
        // this is effectively a move with { 2, 2 }
        cMovedOriginal({ 2, 2 }));
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
    EXPECT_EQ(testedRect.size(), cPoint(16, 16));
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