#include "drcheck/geometry/BoundingBox.h"

BoundingBox::BoundingBox(double minX, double minY, double maxX, double maxY)
	: minX(minX), minY(minY), maxX(maxX), maxY(maxY)
{
}

double BoundingBox::getMinX() const
{
	return minX;
}

double BoundingBox::getMinY() const
{
	return minY;
}

double BoundingBox::getMaxX() const
{
	return maxX;
}

double BoundingBox::getMaxY() const
{
	return maxY;
}

bool BoundingBox::overlaps(const BoundingBox& other) const
{
	return (maxX >= other.minX && minX <= other.maxX && maxY >= other.minY && minY <= other.maxY);
}