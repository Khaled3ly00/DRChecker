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

		double getWidth() const { return maxX - minX; }
		double getHeight() const { return maxY - minY; }
	private:
		double minX;
		double minY;
		double maxX;
		double maxY;
	};
}
