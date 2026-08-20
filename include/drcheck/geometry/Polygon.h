#pragma once

#include <cmath>
#include <cstddef>
#include <vector>
#include <optional>
#include <utility>
#include <optional>
#include <cstddef>

#include "drcheck/geometry/Segment.h"
#include "drcheck/geometry/Point.h"
#include "drcheck/geometry/BoundingBox.h"

namespace drcheck::geometry {
	struct PolygonEdgePairResult
	{
		double distance;

		Point firstPoint;
		Point secondPoint;

		std::size_t firstEdgeIndex;
		std::size_t secondEdgeIndex;
	};
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
		bool contains(const Polygon& other) const;

		bool intersects(const Polygon& other) const;

		PolygonEdgePairResult distanceTo(const Polygon& other, bool treatIntersectionAsZero = true) const;

		PolygonEdgePairResult minWidth() const;
		PolygonEdgePairResult orthogonalMinWidth() const;
	private:
		std::vector<Point> vertices;
		bool hasSelfIntersection() const;
		bool isOrthogonal() const;
		static std::optional<std::pair<double, double>>positiveOverlapInterval(double minA, double maxA, double minB, double maxB);
		double edgePairFacingScore(const Segment& firstEdge, const Segment& secondEdge, const Point& firstPoint, const Point& secondPoint) const;
	};
}
