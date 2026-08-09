#include "drcheck/geometry/Segment.h"
#include "drcheck/geometry/Constants.h"
#include <stdexcept>
#include <algorithm>

namespace drcheck::geometry {
	Segment::Segment(const Point& start, const Point& end)
		: start(start), end(end)
	{
		// Validate that the segment endpoints are distinct points
		if (start == end) {
			throw std::invalid_argument("Segment endpoints must be distinct points.");
		}
	}

	const Point& Segment::getStart() const
	{
		return start;
	}

	const Point& Segment::getEnd() const
	{
		return end;
	}

	BoundingBox Segment::getBoundingBox() const
	{
		double minX = std::min(start.getX(), end.getX());
		double minY = std::min(start.getY(), end.getY());
		double maxX = std::max(start.getX(), end.getX());
		double maxY = std::max(start.getY(), end.getY());
		return BoundingBox(minX, minY, maxX, maxY);
	}

	// Check if point lies on the segment
	bool Segment::contains(const Point& point) const
	{	
		// Point is not on the same line
		if (!(Point::getOrientation(start, end, point) == Orientation::Collinear)) {
			return false;
		}
		// Point is collinear, so check whether it lies
		// between the two endpoints.
		return (point.getX() >= std::min(start.getX(), end.getX()) - EPSILON &&
				point.getX() <= std::max(start.getX(), end.getX()) + EPSILON &&
				point.getY() >= std::min(start.getY(), end.getY()) - EPSILON &&
				point.getY() <= std::max(start.getY(), end.getY()) + EPSILON);
		
	}

	// Check if two segments intersect (Deep check)
	bool Segment::properIntersection(const Segment& other) const
	{
		const Orientation o1 = Point::getOrientation(start, end, other.start); // AB x AC
		const Orientation o2 = Point::getOrientation(start, end, other.end);	// AB x AD
		const Orientation o3 = Point::getOrientation(other.start, other.end, start); // CD x CA
		const Orientation o4 = Point::getOrientation(other.start, other.end, end); // CD x CB
		
		const bool otherCrossesThis =	(o1 == Orientation::CounterClockwise && o2 == Orientation::Clockwise) ||
										(o1 == Orientation::Clockwise && o2 == Orientation::CounterClockwise);
		const bool thisCrossesOther =	(o3 == Orientation::CounterClockwise && o4 == Orientation::Clockwise) ||
										(o3 == Orientation::Clockwise && o4 == Orientation::CounterClockwise);
		return otherCrossesThis && thisCrossesOther;
	}
	// Complete intersection check between two segments, including endpoint touching and collinear overlap.
	bool Segment::intersects(const Segment& other) const
	{
		// First check if bounding boxes overlap (Broad check)
		if (!getBoundingBox().overlaps(other.getBoundingBox())) {
			return false;
		}
		// If bounding boxes overlap, do a deeper check
		// Normal crossing case.
		if (properIntersection(other)) {
			return true;
		}
		// Handle endpoint touching and collinear overlap.
		if (contains(other.start) || contains(other.end) || other.contains(start) || other.contains(end)) {
			return true;
		}
		return false;
	}
	// Calculate the shortest distance from a point to the segment
	double Segment::distanceTo(const Point& point) const
	{
		// Vector from start to end of the segment
		Vector segVector = Point::vectorBetween(start, end);
		// Vector from start to the point
		Vector pointVector = Point::vectorBetween(start, point);
		double segLengthSquared = segVector.dot(segVector);
		// Project pointVector onto segVector and find the projection scalar
		double t = pointVector.dot(segVector) / segLengthSquared;
		if (t < 0.0) {
			// Closest to start point
			return Point::vectorBetween(start, point).length();
		}
		else if (t > 1.0) {
			// Closest to end point
			return Point::vectorBetween(end, point).length();
		}
		else {
			// Projection falls on the segment
			return std::abs(segVector.cross(pointVector)) / segVector.length();
		}
	}
	// Calculate the shortest distance between two segments
	double Segment::distanceTo(const Segment& other) const
	{
		if (intersects(other)) {
			return 0.0; // Segments intersect, so distance is zero
		}
		// Calculate distances from endpoints of one segment to the other segment
		double d1 = distanceTo(other.getStart());
		double d2 = distanceTo(other.getEnd());
		double d3 = other.distanceTo(start);
		double d4 = other.distanceTo(end);
		return std::min({ d1, d2, d3, d4 });
	}
}
