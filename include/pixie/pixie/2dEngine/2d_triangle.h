#pragma once

class c2DTriangle : public c2DRenderable
{
protected:
    std::array<cPoint, 3> mPoints;
public:
    virtual bool GetProperty(unsigned int PropertyFlags, OUT cPropertyValues& Value) const override;
    virtual bool SetProperty(unsigned int PropertyFlags, const cPropertyValues& Value) override;
    virtual cSpriteRenderInfo GetRenderInfo() const override;
    void setPoints(const cPoint& p1, const cPoint& p2, const cPoint& p3);
    void setPoints(const auto& points);
    auto getPoints() const { return mPoints; }
};

void c2DTriangle::setPoints(const auto& points)
{
    ASSERT(points.size() == 3);
    mPoints = points;
}

USE_DROP_INSTEAD_DELETE_PARENT(c2DTriangle, c2DRenderable);


