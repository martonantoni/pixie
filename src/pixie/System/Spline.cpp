#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

// see here: https://en.wikipedia.org/wiki/B%C3%A9zier_curve#Higher-order_curves

cPoint CubicSpline(const cSplineParameters& splineParameters, double t)
{
	if(ASSERTFALSE(splineParameters.size()!=4))
		return {};
	auto a=Lerp(splineParameters[0], splineParameters[1], t);
	auto b=Lerp(splineParameters[1], splineParameters[2], t);
	auto c=Lerp(splineParameters[2], splineParameters[3], t);
	auto d=Lerp(a, b, t);
	auto e=Lerp(b, c, t);
	return Lerp(d, e, t);
}

std::vector<cPoint> GenerateSplinePoints(const cSplineParameters& splineParameters, double DesiredSpacing)
{
    std::vector<cPoint> splinePoints;
    splinePoints.reserve(1000);

    cPoint startPoint = CubicSpline(splineParameters, 0.0);
    splinePoints.push_back(startPoint);

    for(double t=0.0; t<1.0; t+=0.001)
    {
        cPoint point = CubicSpline(splineParameters, t);
        if(point.DistanceFrom(splinePoints.back())>DesiredSpacing)
            splinePoints.push_back(point);        
    }
    splinePoints.emplace_back(CubicSpline(splineParameters, 1.0));

    return splinePoints;
}