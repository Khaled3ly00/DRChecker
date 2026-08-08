#include "drcheck/geometry/Point.h"
#include "drcheck/geometry/Constants.h"
#include <cmath>

namespace drcheck::geometry {
	Point::Point(double x, double y)
		: x(x), y(y)
	{
	}

	double Point::getX() const
	{
		return x;
	}

	double Point::getY() const
	{
		return y;
	}
	Vector Point::vectorBetween(const Point& from, const Point& to) {
		return Vector(to.x - from.x, to.y - from.y);
	}
	double Point::orientationValue(const Point& a, const Point& b, const Point& c) {
		return vectorBetween(a, b).cross(vectorBetween(a, c));
	}
	Orientation Point::getOrientation(const Point& a, const Point& b, const Point& c) {
		double val = orientationValue(a, b, c);
		if (std::abs(val) < EPSILON) return Orientation::Collinear;
		return (val > 0) ? Orientation::CounterClockwise : Orientation::Clockwise;
	}
}
