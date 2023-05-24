#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cPoint CubicSpline(const std::vector<cPoint> &SplineParameters, double t)
{
	if(ASSERTFALSE(SplineParameters.size()!=4))
		return {};
	auto a=Lerp(SplineParameters[0], SplineParameters[1], t);
	auto b=Lerp(SplineParameters[1], SplineParameters[2], t);
	auto c=Lerp(SplineParameters[2], SplineParameters[3], t);
	auto d=Lerp(a, b, t);
	auto e=Lerp(b, c, t);
	return Lerp(d, e, t);
}

std::vector<cPoint> GenerateSplinePoints(const std::vector<cPoint> &SplineParameters, double DesiredSpacing)
{
    RELEASE_ASSERT(SplineParameters.size() == 4);
	constexpr int TableSize=100;
	constexpr double TableResolution=1.0/TableSize;
	std::vector<std::pair<cPoint, double>> SplinePoints;
	SplinePoints.reserve(TableSize);
	SplinePoints.emplace_back(SplineParameters.front(), 0.0);
	for(int i=1; i<TableSize; ++i)
	{
		auto Point=CubicSpline(SplineParameters, TableResolution*i);
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
			Points.emplace_back(CubicSpline(SplineParameters, t));
			DesiredDistance+=DesiredSpacing;
		}
	}
	return Points;
}