#include "drcheck/geometry/Polygon.h"
#include "drcheck/geometry/Constants.h"

#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <limits>

namespace drcheck::geometry {

	Polygon::Polygon(std::vector<Point> vertices)
		: vertices(std::move(vertices)) // Use std::move to efficiently transfer ownership (pointer and value pointing to) of the vertices vector to the Polygon object
	{
		// Validate the polygon: it must have at least 3 vertices,
		// No zero-length edges,
		// The area must be greater than zero (non-degenerate polygon)
		// The polygon must not self-intersect
		if (this->vertices.size() < 3) {
			throw std::invalid_argument("A polygon must have at least 3 vertices.");
		}
		for (std::size_t i = 0; i < this->vertices.size(); ++i)
		{
			const std::size_t next = (i + 1) % this->vertices.size();
			if (this->vertices[i]==(this->vertices[next])) {
				throw std::invalid_argument ("Polygon contains a zero-length edge");
			}
		}
		if (std::abs(signedArea()) < EPSILON) {
			throw std::invalid_argument ("Polygon area must be greater than zero");
		}
		if (hasSelfIntersection()) {
			throw std::invalid_argument ("Polygon must not self-intersect");
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
		// Bounding box check (Broad-phase rejection).
		if (!getBoundingBox().overlaps(BoundingBox(point.getX(), point.getY(), point.getX(), point.getY()))) {
			return false;
		}
		// If the point is within the bounding box, check if it's on the polygon edge.
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
	// Check if two polygons intersect by checking for edge intersections and containment
	bool Polygon::intersects(const Polygon& other) const
	{
		// First check if the bounding boxes (Broad-phase rejection) of the two polygons overlap
		if (!getBoundingBox().overlaps(other.getBoundingBox())) {
			return false;
		}
		// Check for edge intersections between the two polygons
		for (const Segment& edge1 : getEdges()) {
			for (const Segment& edge2 : other.getEdges()) {
				if (edge1.intersects(edge2)) {
					return true;
				}
			}
		}
		// If no edges intersect, check if one polygon is contained within the other
		if (contains(other.getVertices()[0]) || other.contains(getVertices()[0])) {
			return true;
		}
		return false;
	}
	// Calculate the minimum distance between two polygons by checking distances between edges
	double Polygon::distanceTo(const Polygon& other) const
	{
		// If the polygons intersect, the distance is zero
		if (intersects(other)) {
			return 0.0;
		}
		double minDistance = std::numeric_limits<double>::max();
		for (const Segment& edge1 : getEdges()) {
			for (const Segment& edge2 : other.getEdges()) {
				minDistance = std::min(minDistance, edge1.distanceTo(edge2));
			}
		}
		return minDistance;
	}
	// Calculate the minimum width (not minimum edge legnth) of the polygon (currently only supports axis-aligned rectangles)
	double Polygon::minWidth() const
	{
		// Check if the polygon is an axis-aligned rectangle. If not, throw a logic error since minWidth currently only supports axis-aligned rectangles.
		if (!isAxisAlignedRectangle()) {
			throw std::logic_error(
				"minWidth currently supports only "
				"axis-aligned rectangles"
			);
		}
		// Get the edges of the polygon and find the minimum length among them
		const auto polygonEdges = getEdges();

		return std::min(polygonEdges[0].length(), polygonEdges[1].length());
	}

	// Temporary restriction for the initial min-width implementation.
	bool Polygon::isAxisAlignedRectangle() const
	{
		if (vertices.size() != 4) {
			return false;
		}
		// Check if the edges are axis-aligned (horizontal or vertical)
		bool previousHorizontal = false;
		for (std::size_t i = 0; i < vertices.size(); ++i)
		{
			const std::size_t next = (i + 1) % vertices.size();

			const Point& a = vertices[i];
			const Point& b = vertices[next];

			const bool horizontal = std::abs(a.getY() - b.getY()) < EPSILON;

			const bool vertical = std::abs(a.getX() - b.getX()) < EPSILON;
			// Is the edge neither horizontal nor vertical? If so, it's not an axis-aligned rectangle.
			if (!horizontal && !vertical) {
				return false;
			}

			// Zero-length edge:
			if (horizontal && vertical) {
				return false;
			}
			// Check if two consecutive edges are both horizontal or both vertical. If so, it's not an axis-aligned rectangle.
			if (i > 0 && horizontal == previousHorizontal) {
				return false;
			}

			previousHorizontal = horizontal;
		}
		return true;
	}
	// Check if the polygon has self-intersections by checking for intersections between non-adjacent edges
	bool Polygon::hasSelfIntersection() const
	{
		const auto edges = getEdges();
		for (std::size_t i = 0; i < edges.size(); ++i) {
			for (std::size_t j = i + 1; j < edges.size(); ++j) {
				// Skip adjacent edges (they share a vertex)
				if (j == (i + 1) % edges.size() || i == (j + 1) % edges.size()) {
					continue;
				}
				if (edges[i].intersects(edges[j])) {
					return true;
				}
			}
		}
		return false;
	}
}