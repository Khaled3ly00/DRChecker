#pragma once

#include "Point.h"
#include "BoundingBox.h"

namespace drcheck::geometry {
	class Segment
	{
	public:
		Segment(const Point& start, const Point& end);

		const Point& getStart() const;
		const Point& getEnd() const;

		BoundingBox getBoundingBox() const;
		
		bool contains(const Point& point) const;
		bool intersects(const Segment& other) const;
		double distanceTo(const Point& point) const;
		double distanceTo(const Segment& other) const;
	private:
		Point start;
		Point end;

		bool properIntersection(const Segment& other) const; // DEEP INTERSECTION CHECK IF BOUNDING BOXES OVERLAP
	};
}
