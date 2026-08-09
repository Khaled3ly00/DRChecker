#pragma once

namespace drcheck::geometry {
	class BoundingBox
	{
	public:
		BoundingBox(double minX, double minY, double maxX, double maxY);

		bool overlaps(const BoundingBox& other, double tolerance = 0.0) const;

		double getMinX() const;
		double getMinY() const;
		double getMaxX() const;
		double getMaxY() const;

	private:
		double minX;
		double minY;
		double maxX;
		double maxY;
	};
}
