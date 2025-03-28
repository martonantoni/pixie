#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

bool c2DTriangle::GetProperty(unsigned int PropertyFlags, OUT cPropertyValues& Value) const
{
    switch (PropertyFlags)
    {
    case Property_P1: Value = mPoints[0]; return true;
    case Property_P2: Value = mPoints[1]; return true;
    case Property_P3: Value = mPoints[2]; return true;
    }
    return c2DRenderable::GetProperty(PropertyFlags, Value);
}

bool c2DTriangle::SetProperty(unsigned int PropertyFlags, const cPropertyValues& Value)
{
    if (!CheckIfChangableProperty(PropertyFlags))
        return false;
    switch (PropertyFlags)
    {
    case Property_P1: mPoints[0] = Value.ToPoint(); return true;
    case Property_P2: mPoints[1] = Value.ToPoint(); return true;
    case Property_P3: mPoints[2] = Value.ToPoint(); return true;
    }
    return c2DRenderable::SetProperty(PropertyFlags, Value);
}

void c2DTriangle::setPoints(const cPoint& p1, const cPoint& p2, const cPoint& p3)
{
    mPoints[0] = p1;
    mPoints[1] = p2;
    mPoints[2] = p3;
}

cSpriteRenderInfo c2DTriangle::GetRenderInfo() const
{
    return {};
}
