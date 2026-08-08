#pragma once

#include <vector>

#include "drcheck/geometry/Segment.h"
#include "drcheck/geometry/Point.h"
#include "drcheck/geometry/BoundingBox.h"

namespace drcheck::geometry {
	class Polygon
	{
	public:
		explicit Polygon(std::vector<Point> vertices); // explicit constructor to prevent implicit conversions (from std::vector<Point> to Polygon) if the user passes a vector of points to the function.
		const std::vector<Point>& getVertices() const;
		std::size_t getVertexCount() const { return vertices.size(); }

		std::vector<Segment> getEdges() const;
		double signedArea() const;
		double area() const { return std::abs(signedArea()); }
		Orientation getOrientation() const;
		BoundingBox getBoundingBox() const;
		bool contains(const Point& point) const;
		bool intersects(const Polygon& other) const;
	private:
		std::vector<Point> vertices;
	};
}
