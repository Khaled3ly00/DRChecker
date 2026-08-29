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
			if (this->vertices[i].isNear(this->vertices[next])) {
				throw std::invalid_argument ("Polygon contains a zero-length edge");
			}
		}
		double boundaryLength = 0.0;
		for (const Segment& edge : getEdges()) {
			boundaryLength += edge.length();
		}

		const double areaTolerance = EPSILON * boundaryLength;
		if (std::abs(signedArea()) <= areaTolerance) {
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
		edges.reserve(vertices.size());
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
		return signedArea() > 0.0
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

	bool Polygon::contains(const Point& point, bool includeBoundary) const
	{
		// Bounding box check (Broad-phase rejection).
		if (!getBoundingBox().overlaps(
				BoundingBox(
					point.getX(),
					point.getY(),
					point.getX(),
					point.getY()
				),
				EPSILON)) {
			return false;
		}
		// If the point is within the bounding box, check if it's on the polygon edge.
		for (const Segment& edge : getEdges()) {
			if (edge.contains(point)) {
				return includeBoundary;
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
	// Checks whether this polygon completely contains the other polygon.
	bool Polygon::contains(const Polygon& other) const
	{	
		//  Every vertex of the other polygon must lie inside this polygon.
		for (const Point& vertex : other.getVertices()) {
			if (!contains(vertex)) {
				return false;
			}
		}
		// Strict containment does not allow the two polygon boundaries to cross (touching not counted as intersection).
		// Tailored to MinEnclosure as inner polygon touching is considered contained
		const auto thisEdges = getEdges();
		const auto otherEdges = other.getEdges();

		for (const Segment& edge1 : thisEdges) {
			for (const Segment& edge2 : otherEdges) {
				if (edge1.intersects(edge2, false)) {
					return false;
				}
			}
		}
		return true;
	}
	// Check if two polygons intersect by checking for edge intersections and containment
	bool Polygon::intersects(const Polygon& other) const
	{
		// First check if the bounding boxes (Broad-phase rejection) of the two polygons overlap
		if (!getBoundingBox().overlaps(other.getBoundingBox(), EPSILON)) {
			return false;
		}
		// Check for edge intersections between the two polygons
		const auto thisEdges = getEdges();
		const auto otherEdges = other.getEdges();

		for (const Segment& edge1 : thisEdges) {
			for (const Segment& edge2 : otherEdges) {
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

	// Checks if both polygons have shared edge (shared vertex returns false)
	bool Polygon::sharesBoundarySegment(const Polygon& other) const {
		// Broad Phase Check
		if (!getBoundingBox().overlaps(other.getBoundingBox(), EPSILON))
		{
			return false;
		}

		const auto thisEdges = getEdges();
		const auto otherEdges = other.getEdges();

		for (const Segment& firstEdge : thisEdges)
		{
			for (const Segment& secondEdge : otherEdges)
			{
				// Check if the two edges collinear
				const bool collinear =
					Point::getOrientation(firstEdge.getStart(), firstEdge.getEnd(), secondEdge.getStart()) == Orientation::Collinear &&
					Point::getOrientation(firstEdge.getStart(), firstEdge.getEnd(), secondEdge.getEnd()) == Orientation::Collinear;

				if (!collinear)
				{
					continue;
				}

				// If collinear, check for overlap distance
				const double dx = std::abs(firstEdge.getEnd().getX() - firstEdge.getStart().getX());
				const double dy = std::abs(firstEdge.getEnd().getY() - firstEdge.getStart().getY());
				if (dx >= dy)
				{
					const auto overlap = positiveOverlapInterval(
						std::min(firstEdge.getStart().getX(), firstEdge.getEnd().getX()),
						std::max(firstEdge.getStart().getX(), firstEdge.getEnd().getX()),
						std::min(secondEdge.getStart().getX(), secondEdge.getEnd().getX()),
						std::max(secondEdge.getStart().getX(), secondEdge.getEnd().getX())
					);

					if (overlap)
					{
						return true;
					}
				}
				else
				{
					const auto overlap = positiveOverlapInterval(
						std::min(firstEdge.getStart().getY(), firstEdge.getEnd().getY()),
						std::max(firstEdge.getStart().getY(), firstEdge.getEnd().getY()),
						std::min(secondEdge.getStart().getY(), secondEdge.getEnd().getY()),
						std::max(secondEdge.getStart().getY(), secondEdge.getEnd().getY())
					);

					if (overlap)
					{
						return true;
					}
				}
			}
		}
		return false;
	}
	// Check if two polygons have shared area
	bool Polygon::overlaps(const Polygon& other) const {
		// Broad Phase Check
		if (!getBoundingBox().overlaps(other.getBoundingBox(), EPSILON))
		{
			return false;
		}
		
		const auto thisEdges = getEdges();
		const auto otherEdges = other.getEdges();

		// Check for edge intersection (false: don't include endpoints touching)
		for (const Segment& firstEdge : thisEdges)
		{
			for (const Segment& secondEdge : otherEdges)
			{
				if (firstEdge.intersects(secondEdge, false))
				{
					return true;
				}
			}
		}

		// Check if one polygon completely contains other
		if (contains(other) || other.contains(*this))
		{
			return true;
		}

		// Check for partial enclosure
		for (const Point& vertex : vertices)
		{
			if (other.contains(vertex, false))
			{
				return true;
			}
		}

		for (const Point& vertex : other.getVertices())
		{
			if (contains(vertex, false))
			{
				return true;
			}
		}

		for (const Segment& edge : thisEdges)
		{
			const Point midpoint(
				(edge.getStart().getX() + edge.getEnd().getX()) / 2.0,
				(edge.getStart().getY() + edge.getEnd().getY()) / 2.0
			);

			if (other.contains(midpoint, false))
			{
				return true;
			}
		}

		for (const Segment& edge : otherEdges)
		{
			const Point midpoint(
				(edge.getStart().getX() + edge.getEnd().getX()) / 2.0,
				(edge.getStart().getY() + edge.getEnd().getY()) / 2.0
			);

			if (contains(midpoint, false))
			{
				return true;
			}
		}
		return false;
	}

	// Calculate the minimum distance between two polygons by checking distances between edges
	PolygonEdgePairResult Polygon::distanceTo(const Polygon& other, bool treatIntersectionAsZero) const
	{	
		const auto thisEdges = getEdges();
		const auto otherEdges = other.getEdges();
		const DistanceResult initialDistance = thisEdges[0].distanceTo(otherEdges[0]);

		PolygonEdgePairResult best{initialDistance.distance, initialDistance.firstPoint, initialDistance.secondPoint, 0, 0};

		for (std::size_t i = 0; i < thisEdges.size(); ++i) {
			for (std::size_t j = 0; j < otherEdges.size(); ++j) {
				const DistanceResult result = thisEdges[i].distanceTo(otherEdges[j]);
				if (result.distance + EPSILON < best.distance) {
					best = {result.distance, result.firstPoint, result.secondPoint, i, j};
				}
				// If there's multiple edges pair with same spacing
				else if (std::abs(result.distance - best.distance) <= EPSILON) {
					if (result.distance <= EPSILON)
					{
						continue;
					}
					// Calculate candidate pair score
					const double candidateScore = edgePairFacingScore(thisEdges[i], otherEdges[j], result.firstPoint, result.secondPoint);
					// Calculate best pair score
					const double bestScore = edgePairFacingScore(thisEdges[best.firstEdgeIndex], otherEdges[best.secondEdgeIndex], best.firstPoint, best.secondPoint);
					// Lower score wins (more perpendicular to the connector pair)
					if (candidateScore + EPSILON < bestScore)
					{
						best = { result.distance, result.firstPoint, result.secondPoint, i, j };
					}
				}
			}
		}
		// For ordinary distance, overlapping or contained polygons have zero distance.
		if (treatIntersectionAsZero && intersects(other))
		{
			best.distance = 0.0;
		}
		return best;
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
	// Calculate the minimum local width of the polygon.
	// Temporary restriction: This implementation is only valid for orthogonal polygons (polygons with edges aligned to the axes).
	// Will be extended to handle non-orthogonal polygons (45 degrees) in the future.
	PolygonEdgePairResult Polygon::minWidth() const
	{
		if (isOrthogonal()) {
			return orthogonalMinWidth();
		}

		throw std::logic_error(
			"Minimum width for non-orthogonal "
			"polygons is not implemented yet"
		);
	}
	PolygonEdgePairResult Polygon::orthogonalMinWidth() const
	{
		const auto polygonEdges = getEdges();

		std::optional<PolygonEdgePairResult> bestResult;

		for (std::size_t i = 0; i < polygonEdges.size(); ++i)
		{
			for (std::size_t j = i + 1; j < polygonEdges.size(); ++j)
			{
				const Segment& first = polygonEdges[i];
				const Segment& second = polygonEdges[j];
				// Horizontal edge pair
				if (first.isHorizontal() && second.isHorizontal()) {
					const double firstMinX = std::min(first.getStart().getX(), first.getEnd().getX());
					const double firstMaxX = std::max(first.getStart().getX(), first.getEnd().getX());
					const double secondMinX = std::min(second.getStart().getX(), second.getEnd().getX());
					const double secondMaxX = std::max(second.getStart().getX(), second.getEnd().getX());

					const auto overlap = positiveOverlapInterval(firstMinX, firstMaxX, secondMinX, secondMaxX);
					// Is there a positive overlap interval between the two horizontal edges? If not, skip this pair.
					if (!overlap) {
						continue;
					}

					const double firstY = first.getStart().getY();
					const double secondY = second.getStart().getY();
					const double width = std::abs(firstY - secondY);

					// Ignore coincident or tolerance-close boundaries.

					if (width <= EPSILON) {
						continue;
					}

					const double sampleX = (overlap->first + overlap->second) / 2.0;
					const double sampleY = (firstY + secondY) / 2.0;

					const Point samplePoint(sampleX, sampleY);

					// The region between the two boundaries
					// must lie inside the polygon.
					if (!contains(samplePoint)) {
						continue;
					}
					// Points lying on violating edges
					const Point firstPoint(sampleX, firstY);
					const Point secondPoint(sampleX, secondY);

					if (!bestResult.has_value() || width < bestResult->distance)
					{
						bestResult = PolygonEdgePairResult{width, firstPoint, secondPoint, i, j};
					}
				}

				// Vertical edge pair
				else if (first.isVertical() && second.isVertical()) {
					const double firstMinY = std::min(first.getStart().getY(), first.getEnd().getY());
					const double firstMaxY = std::max(first.getStart().getY(), first.getEnd().getY());
					const double secondMinY = std::min(second.getStart().getY(), second.getEnd().getY());
					const double secondMaxY = std::max(second.getStart().getY(), second.getEnd().getY());

					const auto overlap = positiveOverlapInterval(firstMinY, firstMaxY, secondMinY, secondMaxY);

					if (!overlap) {
						continue;
					}

					const double firstX = first.getStart().getX();
					const double secondX = second.getStart().getX();
					const double width = std::abs(firstX - secondX);

					if (width <= EPSILON) {
						continue;
					}

					const double sampleY = (overlap->first + overlap->second) / 2.0;
					const double sampleX = (firstX + secondX) / 2.0;
					const Point samplePoint(sampleX, sampleY);

					if (!contains(samplePoint)) {
						continue;
					}
					const Point firstPoint(firstX, sampleY);
					const Point secondPoint(secondX, sampleY);

					if (!bestResult.has_value() || width < bestResult->distance)
					{
						bestResult = PolygonEdgePairResult{width, firstPoint, secondPoint, i, j};
					}
				}
			}
		}

		if (!bestResult.has_value())
		{
			throw std::logic_error(
				"Unable to determine polygon minimum width"
			);
		}

		return bestResult.value();
	}

	// Check if the polygon is orthogonal (all edges are either horizontal or vertical)
	bool Polygon::isOrthogonal() const
	{
		const auto polygonEdges = getEdges();

		for (const Segment& edge : polygonEdges)
		{
			if (!edge.isHorizontal() && !edge.isVertical())
			{
				return false;
			}
		}
		return true;
	}
	// Calculate the positive overlap interval between two ranges [minA, maxA] and [minB, maxB].
	std::optional<std::pair<double, double>>Polygon::positiveOverlapInterval(double minA, double maxA, double minB, double maxB)
	{
		const double overlapMin = std::max(minA, minB);
		const double overlapMax = std::min(maxA, maxB);

		// No meaningful overlap.
		// A single-point touch is not considered
		// a positive overlap interval.
		if (overlapMax - overlapMin <= EPSILON) {
			return std::nullopt;
		}

		return std::make_pair(overlapMin, overlapMax);
	}
	// Calculates a score that represents how much these pair of edges are perpendicular to connector 
	// (0: perpendicular, 2: parallel)
	// Used to select correct violating Min Spacing edge pair,
	// Where lower score is better pair
	double Polygon::edgePairFacingScore(const Segment& firstEdge, const Segment& secondEdge, const Point& firstPoint, const Point& secondPoint) const {
	
		const Vector connector = Point::vectorBetween(firstPoint, secondPoint);
		const Vector firstEdgeVector = Point::vectorBetween(firstEdge.getStart(), firstEdge.getEnd());
		const Vector secondEdgeVector = Point::vectorBetween(secondEdge.getStart(), secondEdge.getEnd());

		const double connectorLength = connector.length();
		// A zero-length connector has no meaningful direction,
		// so a facing score cannot be calculated.
		if (connectorLength <= EPSILON)
		{
			return 0.0;
		}
		const double firstEdgeScore = std::abs(firstEdgeVector.dot(connector)) / (firstEdgeVector.length() * connectorLength);
		const double secondEdgeScore = std::abs(secondEdgeVector.dot(connector)) / (secondEdgeVector.length() * connectorLength);

		return firstEdgeScore + secondEdgeScore;
	}
	// Sutherland-Hodgman helper
	bool Polygon::isPointInsideWindow(const Point& point, const BoundingBox& window, ClipBoundary boundary) const
	{
		switch (boundary)
		{
		case ClipBoundary::Left:
			return point.getX() >= window.getMinX();

		case ClipBoundary::Right:
			return point.getX() <= window.getMaxX();

		case ClipBoundary::Bottom:
			return point.getY() >= window.getMinY();

		case ClipBoundary::Top:
			return point.getY() <= window.getMaxY();
		}
		throw std::logic_error("Unknown window clipping boundary");
	}
	// Sutherland-Hodgman helper
	Point Polygon::intersectionPointWithWindowBoundary(const Point& first, const Point& second, const BoundingBox& window, ClipBoundary boundary) const
	{
		const double dx = second.getX() - first.getX();
		const double dy = second.getY() - first.getY();

		switch (boundary)
		{
		case ClipBoundary::Left:
		{
			const double x = window.getMinX();
			const double t = (x - first.getX()) / dx;
			return Point(x, first.getY() + t * dy);
		}
		case ClipBoundary::Right:
		{
			const double x = window.getMaxX();
			const double t = (x - first.getX()) / dx;
			return Point(x, first.getY() + t * dy);
		}
		case ClipBoundary::Bottom:
		{
			const double y = window.getMinY();
			const double t = (y - first.getY()) / dy;
			return Point(first.getX() + t * dx, y);
		}
		case ClipBoundary::Top:
		{
			const double y = window.getMaxY();
			const double t = (y - first.getY()) / dy;
			return Point(first.getX() + t * dx, y);
		}
		}
		throw std::logic_error("Unknown window clipping boundary");
	}
	// Sutherland-Hodgman Algorithm
	// input: polygon vertices		window: Scan Window		boundary: Scan Window Boundary
	std::vector<Point> Polygon::clipAgainstWindowBoundary(const std::vector<Point>& input, const BoundingBox& window, ClipBoundary boundary) const
	{
		std::vector<Point> output;

		if (input.empty())
		{
			return output;
		}
		Point previous = input.back();
		bool previousInside = isPointInsideWindow(previous, window, boundary);

		for (const Point& current : input)
		{
			const bool currentInside = isPointInsideWindow(current, window, boundary);
			// Inside -> Inside
			if (previousInside && currentInside)
			{
				output.push_back(current);
			}
			// Inside -> Outside
			else if (previousInside && !currentInside)
			{
				output.push_back(intersectionPointWithWindowBoundary(previous, current, window, boundary));
			}
			// Outside -> Inside
			else if (!previousInside && currentInside)
			{
				output.push_back(intersectionPointWithWindowBoundary(previous, current, window, boundary));
				output.push_back(current);
			}
			previous = current;
			previousInside = currentInside;
		}
		return output;
	}
	// Returns area of current polygon object that's within window
	double Polygon::areaInsideWindow(const BoundingBox& window) const {
		if (!getBoundingBox().overlaps(window))
		{
			return 0.0;
		}
		std::vector<Point> clipped = vertices;
		clipped = clipAgainstWindowBoundary(clipped, window,ClipBoundary::Left);
		clipped = clipAgainstWindowBoundary(clipped, window, ClipBoundary::Right);
		clipped = clipAgainstWindowBoundary(clipped, window, ClipBoundary::Bottom);
		clipped = clipAgainstWindowBoundary(clipped, window, ClipBoundary::Top);
		if (clipped.size() < 3)
		{
			return 0.0;
		}
		// Shoelace Formula. Don't use Polygon.signedArea() as polygon may not be constructed due to violation (less than 3 points, .. etc)
		double twiceArea = 0.0;

		for (std::size_t i = 0; i < clipped.size(); ++i)
		{
			const Point& current = clipped[i];
			const Point& next = clipped[(i + 1) % clipped.size()];

			twiceArea += current.getX() * next.getY() - next.getX() * current.getY();
		}

		return std::abs(twiceArea) / 2.0;
	}
}
