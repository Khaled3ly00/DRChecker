#pragma once

namespace drcheck::geometry {
	class BoundingBox
	{
	public:
		BoundingBox(double minX, double minY, double maxX, double maxY);

		bool overlaps(const BoundingBox& other) const;

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
