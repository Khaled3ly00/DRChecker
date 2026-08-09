#pragma once

#include "Vector.h"

namespace drcheck::geometry {
	enum class Orientation {
		Clockwise,
		CounterClockwise,
		Collinear
	};

	class Point
	{
	public:
		Point(double x, double y);

		double getX() const;
		double getY() const;
		// Overload the equality operator to compare two points
		bool operator==(const Point& other) const;

		// STATIC METHODS (UTILITY FUNCTIONS)
		static Vector vectorBetween(const Point& from, const Point& to);

		static double orientationValue(const Point& a, const Point& b, const Point& c);

		static Orientation getOrientation(const Point& a, const Point& b, const Point& c);
	private:
		double x;
		double y;
	};
}
