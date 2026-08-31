#pragma once

#include <cmath>
#include <cstddef>
#include <vector>
#include <optional>
#include <utility>
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

	struct PairwiseEnclosureResult
	{
		double left;
		double right;
		double bottom;
		double top;
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

		bool contains(const Point& point, bool includeBoundary = true) const;
		bool contains(const Polygon& other, bool includeBoundary = false) const;

		bool intersects(const Polygon& other) const;

		bool sharesBoundarySegment(const Polygon& other) const;

		bool overlaps(const Polygon& other) const;

		PolygonEdgePairResult distanceTo(const Polygon& other, bool treatIntersectionAsZero = true) const;

		PolygonEdgePairResult minWidth() const;
		PolygonEdgePairResult orthogonalMinWidth() const;

		double areaInsideWindow(const BoundingBox& region) const;

		PairwiseEnclosureResult pairwiseEnclosure(const Polygon& outerPolygon) const;
	private:
		enum class ClipBoundary
		{
			Left,
			Right,
			Bottom,
			Top
		};
		std::vector<Point> vertices;
		bool hasSelfIntersection() const;
		bool isOrthogonal() const;
		bool isAxisAlignedRectangle() const;
		static std::optional<std::pair<double, double>>positiveOverlapInterval(double minA, double maxA, double minB, double maxB);
		double edgePairFacingScore(const Segment& firstEdge, const Segment& secondEdge, const Point& firstPoint, const Point& secondPoint) const;
		bool isPointInsideWindow(const Point& point, const BoundingBox& window, ClipBoundary boundary) const;
		Point intersectionPointWithWindowBoundary(const Point& first, const Point& second, const BoundingBox& window, ClipBoundary boundary) const;
		std::vector<Point> clipAgainstWindowBoundary(const std::vector<Point>& input, const BoundingBox& region, ClipBoundary boundary) const;
	};
}
