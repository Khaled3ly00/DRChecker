#include "drcheck/rules/MinSpacingRule.h"
#include "drcheck/rules/Constants.h"
#include "drcheck/geometry/Polygon.h"
#include "drcheck/spatial/LayerSpatialIndex.h"

#include <stdexcept>

namespace drcheck::rules {
MinSpacingRule::MinSpacingRule(domain::Layer layer, double minimumSpacing)
    : layer(layer), minimumSpacing(minimumSpacing)
{
    if (minimumSpacing <= 0.0) {
        throw std::invalid_argument(
            "Minimum spacing must be positive"
        );
    }
}
std::vector<domain::Violation> MinSpacingRule::check(const std::vector<domain::Shape>& shapes, const spatial::LayerSpatialIndex& spatialIndex) const
{
    std::vector<domain::Violation> violations;

    for (const domain::Shape& firstShape : shapes) {
        // Confirm first shape follows rule layer
        if (firstShape.getLayer() != layer)
        {
            continue;
        }
        const geometry::BoundingBox searchRegion = firstShape.getPolygon().getBoundingBox().expanded(minimumSpacing);
        // Query Expanded Bounding Box For Shapes Within Same Layer
        const auto candidates = spatialIndex.query(layer, searchRegion);
        
        for (const domain::Shape* secondShape : candidates)
        {
            // Avoid checking the shape against itself.
            if (secondShape == &firstShape) {
                continue;
            }

            // Prevent duplicate A-B / B-A checks.
            // Shapes have unique IDs arranged ascendingly 
            if (secondShape->getId() <= firstShape.getId())
            {
                continue;
            }

            const geometry::PolygonEdgePairResult actualSpacing = firstShape.getPolygon().distanceTo(secondShape->getPolygon());

            if (actualSpacing.distance + DRC_LENGTH_TOLERANCE < minimumSpacing)
            {
                domain::ViolationMarker marker{
                    .firstPoint = actualSpacing.firstPoint,
                    .secondPoint = actualSpacing.secondPoint,
                    .firstLayer = getLayer()
                };
                const std::string msg = "Minimum spacing violation on layer: " + domain::layerToString(layer) + "should be " + std::to_string(minimumSpacing) + " actual" + std::to_string(actualSpacing.distance);

                violations.emplace_back(domain::ViolationType::MinSpacing, std::vector<std::size_t>{firstShape.getId(), secondShape->getId()},
                   msg, actualSpacing.distance, minimumSpacing, marker);
            }
        }
    }
    return violations;
}
domain::Layer MinSpacingRule::getLayer() const
{
    return layer;
}
double MinSpacingRule::getMinimumSpacing() const
{
	return minimumSpacing;
}
}