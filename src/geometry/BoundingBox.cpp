#include "drcheck/geometry/BoundingBox.h"

#include <stdexcept>

namespace drcheck::geometry {
	BoundingBox::BoundingBox(double minX, double minY, double maxX, double maxY)
		: minX(minX), minY(minY), maxX(maxX), maxY(maxY)
	{
		if (minX > maxX || minY > maxY) {
			throw std::invalid_argument(
				"BoundingBox minimum coordinates must not exceed maximum coordinates"
			);
		}
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
	// Overlaps is a symmetric function that checks if any of two bounding boxes have common area
	bool BoundingBox::overlaps(const BoundingBox& other, double tolerance) const
	{
		if (tolerance < 0.0) {
			throw std::invalid_argument(
				"BoundingBox overlap tolerance cannot be negative"
			);
		}

		return
			maxX + tolerance >= other.minX &&
			minX - tolerance <= other.maxX &&
			maxY + tolerance >= other.minY &&
			minY - tolerance <= other.maxY;
	}
	// Contains is NOT a symmetric function it checks if a bounding box contains other
	bool BoundingBox::contains(const BoundingBox& other) const
	{
		return
			other.minX >= minX &&
			other.maxX <= maxX &&
			other.minY >= minY &&
			other.maxY <= maxY;
	}
	// Returns the smallest bounding box that contains both bounding boxes.
	BoundingBox BoundingBox::mergedWith(const BoundingBox& other) const
	{
		return BoundingBox(
			std::min(minX, other.minX),
			std::min(minY, other.minY),
			std::max(maxX, other.maxX),
			std::max(maxY, other.maxY)
		);
	}
	// Return an expanded bounding box for quadtree queries
	BoundingBox BoundingBox::expanded(double amount) const
	{
		if (amount < 0.0)
		{
			throw std::invalid_argument("Bounding box expansion cannot be negative");
		}
		return BoundingBox(
			minX - amount,
			minY - amount,
			maxX + amount,
			maxY + amount
		);
	}
}
