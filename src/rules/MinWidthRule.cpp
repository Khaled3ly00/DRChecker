#include "drcheck/rules/MinWidthRule.h"
#include "drcheck/rules/Constants.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace drcheck::rules {

MinWidthRule::MinWidthRule(domain::Layer layer, double minimumWidth)
	: layer(layer), minimumWidth(minimumWidth) {
	// Validate the minimum width
	if (minimumWidth <= 0) {
		throw std::invalid_argument("Minimum width must be positive.");
	}
}

// Checks the shapes for violations of the minimum width rule
std::vector<domain::Violation> MinWidthRule::check(const std::vector<domain::Shape>& shapes, const spatial::LayerSpatialIndex&) const {
	std::vector<domain::Violation> violations; // Create a vector to hold any violations found
	// Iterate through each shape and check if it violates the minimum width rule
	for (const domain::Shape& shape : shapes)
	{
		// Only check shapes on the specified layer
		if (shape.getLayer() != layer) {
			continue;
		}
		const geometry::PolygonEdgePairResult actualWidth = shape.getPolygon().minWidth();
		if (actualWidth.distance + DRC_LENGTH_TOLERANCE < minimumWidth) {
			domain::ViolationMarker marker{
				.firstPoint = actualWidth.firstPoint,
				.secondPoint = actualWidth.secondPoint,
				.firstEdgeIndex = actualWidth.firstEdgeIndex,
				.secondEdgeIndex = actualWidth.secondEdgeIndex,
				.firstLayer = getLayer()
			};
			const std::string msg = "Minimum width violation on layer: " + domain::layerToString(layer) + "should be " + std::to_string(minimumWidth) + " actual" + std::to_string(actualWidth.distance);
			// If the shape's width is less than the minimum width, create a violation (object) and add it to the violations vector
			violations.emplace_back(domain::ViolationType::MinWidth, std::vector<std::size_t>{shape.getId()}, std::move(msg), actualWidth.distance, minimumWidth, marker);
		}
	}
	return violations;
}
double MinWidthRule::getMinimumWidth() const {
	return minimumWidth;
}
domain::Layer MinWidthRule::getLayer() const {
	return layer;
}
}