#pragma once

#include "Point.h"
#include "BoundingBox.h"

namespace drcheck::geometry {
	// First Point is point on "this" segment
	// Second Point is point on "other" segment
	struct DistanceResult
	{
		double distance;
		Point firstPoint;
		Point secondPoint;
	};
	class Segment
	{
	public:
		Segment(const Point& start, const Point& end);

		const Point& getStart() const;
		const Point& getEnd() const;
		double length() const { return Point::vectorBetween(start, end).length(); }

		BoundingBox getBoundingBox() const;
		bool isHorizontal() const { return start.getY() == end.getY(); }
		bool isVertical() const { return start.getX() == end.getX(); }

		bool contains(const Point& point) const;
		bool intersects(const Segment& other, bool includeBoundaryContact = true) const;
		DistanceResult distanceTo(const Point& point) const;
		DistanceResult distanceTo(const Segment& other) const;
	private:
		Point start;
		Point end;

		bool properIntersection(const Segment& other) const; // DEEP INTERSECTION CHECK IF BOUNDING BOXES OVERLAP
	};
}
