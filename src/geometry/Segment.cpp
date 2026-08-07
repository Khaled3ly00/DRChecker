#include "drcheck/geometry/Segment.h"

#include <algorithm>

Segment::Segment(const Point& start, const Point& end)
	: start(start), end(end)
{
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