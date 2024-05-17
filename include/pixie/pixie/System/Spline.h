#pragma once

using cSplineParameters = std::array<cPoint, 4>;

cPoint CubicSpline(const cSplineParameters& splineParameters, double t);

std::vector<cPoint> GenerateSplinePoints(const cSplineParameters& splineParameters, double RequiredSpacing); 
