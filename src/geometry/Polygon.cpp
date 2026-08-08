#include "drcheck/geometry/Polygon.h"
#include "drcheck/geometry/Constants.h"

#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <utility>

namespace drcheck::geometry {

	Polygon::Polygon(std::vector<Point> vertices)
		: vertices(std::move(vertices)) // Use std::move to efficiently transfer ownership (pointer and value pointing to) of the vertices vector to the Polygon object
	{
		if (this->vertices.size() < 3) {
			throw std::invalid_argument("A polygon must have at least 3 vertices.");
		}
	}

	const std::vector<Point>& Polygon::getVertices() const
	{
		return vertices;
	}

	// Get the edges of the polygon as segments from the vertices
	std::vector<Segment> Polygon::getEdges() const
	{
		std::vector<Segment> edges;
		size_t n = vertices.size();
		for (size_t i = 0; i < n; ++i) {
			edges.emplace_back(vertices[i], vertices[(i + 1) % n]);
		}
		return edges;

	}

	// Calculate the signed area of the polygon using the shoelace formula
	double Polygon::signedArea() const
	{
		double area = 0.0;
		size_t n = vertices.size();
		for (size_t i = 0; i < n; ++i) {
			const Point& p1 = vertices[i];
			const Point& p2 = vertices[(i + 1) % n];
			area += (p1.getX() * p2.getY()) - (p2.getX() * p1.getY());
		}
		return area / 2.0;
	}

	// Determine the orientation of the polygon based on the signed area (positive vertices are counter-clockwise, negative vertices are clockwise)
	Orientation Polygon::getOrientation() const
	{
		double area = signedArea();
		if (std::abs(area) < EPSILON) {
			return Orientation::Collinear;
		}

		return area > 0.0
			? Orientation::CounterClockwise
			: Orientation::Clockwise;
	}

	// Get the bounding box of the polygon by finding the min and max x and y coordinates of the vertices
	BoundingBox Polygon::getBoundingBox() const
	{
		double minX = vertices[0].getX();
		double maxX = vertices[0].getX();
		double minY = vertices[0].getY();
		double maxY = vertices[0].getY();
		for (const auto& vertex : vertices) {
			minX = std::min(minX, vertex.getX());
			maxX = std::max(maxX, vertex.getX());
			minY = std::min(minY, vertex.getY());
			maxY = std::max(maxY, vertex.getY());
		}
		return BoundingBox(minX, minY, maxX, maxY);
	}

	bool Polygon::contains(const Point& point) const
	{
		// First handle points exactly on the polygon edge.
		for (const Segment& edge : getEdges()) {
			if (edge.contains(point)) {
				return true;
			}
		}
		// Use the ray-casting algorithm to determine if the point is inside the polygon
		bool inside = false;
		size_t n = getVertexCount();
		for (size_t i = 0; i < n; i++) {
			const Point& a = vertices[i];
			const Point& b = vertices[(i + 1) % n];
			// Check if the point is within the y-range of the edge
			bool crossesY = ((a.getY() > point.getY()) != (b.getY() > point.getY()));
			if (!crossesY) {
				continue;
			}
			// Calculate the x-coordinate of the intersection of the edge with the horizontal line at point.getY()
			const double intersectionX =
				a.getX() +
				(point.getY() - a.getY()) *
				(b.getX() - a.getX()) /
				(b.getY() - a.getY());
			// If the intersection is to the right of the point (Horizontal ray passes the edge), toggle the inside status (even-odd rule)
			if (intersectionX > point.getX()) {
				inside = !inside;
			}
		}
		return inside;
	}
}