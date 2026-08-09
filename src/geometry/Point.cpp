#include "drcheck/geometry/Point.h"
#include "drcheck/geometry/Constants.h"

#include <algorithm>
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
	bool Point::isNear(const Point& other) const
	{
		return
			std::abs(x - other.x) < EPSILON &&
			std::abs(y - other.y) < EPSILON;
	}
	Vector Point::vectorBetween(const Point& from, const Point& to) {
		return Vector(to.x - from.x, to.y - from.y);
	}
	double Point::orientationValue(const Point& a, const Point& b, const Point& c) {
		return vectorBetween(a, b).cross(vectorBetween(a, c));
	}
	Orientation Point::getOrientation(const Point& a, const Point& b, const Point& c) {
		const Vector ab = vectorBetween(a, b);
		const Vector ac = vectorBetween(a, c);
		const Vector bc = vectorBetween(b, c);

		const double scale = std::max({
			ab.length(),
			ac.length(),
			bc.length()
		});

		if (scale < EPSILON) {
			return Orientation::Collinear;
		}

		const double value = ab.cross(ac);
		const double orientationTolerance = EPSILON * scale;

		if (std::abs(value) <= orientationTolerance) {
			return Orientation::Collinear;
		}

		return value > 0.0
			? Orientation::CounterClockwise
			: Orientation::Clockwise;
	}
}
