#pragma once

cPoint CubicSpline(const std::vector<cPoint> &SplineParameters, double t);

std::vector<cPoint> GenerateSplinePoints(const std::vector<cPoint> &SplineParameters, double RequiredSpacing); // needs 4 points
