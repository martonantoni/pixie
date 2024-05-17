#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

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
    RELEASE_ASSERT(splineParameters.size() == 4);
	constexpr int TableSize=100;
	constexpr double TableResolution=1.0/TableSize;
	std::vector<std::pair<cPoint, double>> SplinePoints;
	SplinePoints.reserve(TableSize);
	SplinePoints.emplace_back(splineParameters.front(), 0.0);
	for(int i=1; i<TableSize; ++i)
	{
		auto Point=CubicSpline(splineParameters, TableResolution*i);
		SplinePoints.emplace_back(Point, Point.DistanceFrom(SplinePoints.back().first)+SplinePoints.back().second);
	}
	double DesiredDistance=DesiredSpacing;
	std::vector<cPoint> Points;
	Points.emplace_back(SplinePoints.front().first);
	for(int i=1;i<TableSize;++i)
	{
		while(SplinePoints[i].second>DesiredDistance)
		{
			// will be between index i and i-1
			double DistanceFromPrev=SplinePoints[i].second-SplinePoints[i-1].second;
			if(DistanceFromPrev<0.00000001)
				continue;
			double t=i*TableResolution-TableResolution*((SplinePoints[i].second-DesiredDistance)/DistanceFromPrev);
			Points.emplace_back(CubicSpline(splineParameters, t));
			DesiredDistance+=DesiredSpacing;
		}
	}
	return Points;
}