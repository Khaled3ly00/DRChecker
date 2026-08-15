#include "drcheck/rules/MinSpacingRule.h"
#include "drcheck/spatial/Quadtree.h"
#include "drcheck/geometry/Constants.h"

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
std::vector<domain::Violation> MinSpacingRule::check(const std::vector<domain::Shape>& shapes) const
{
    std::vector<domain::Violation> violations;
    const domain::Shape* firstTargetShape = nullptr;

    // Check if there's any shape with the same layer
    for (const domain::Shape& shape : shapes) {
        if (shape.getLayer() == layer) {
            firstTargetShape = &shape;
            break;
        }
    }
    if (!firstTargetShape) {
        return violations;
    }
    // Merge all bounding boxes for shapes with same layer to get root boundary
    geometry::BoundingBox boundary = firstTargetShape->getPolygon().getBoundingBox();
    for (const domain::Shape& shape : shapes) {
        if (shape.getLayer() != layer) {
            continue;
        }
        boundary = boundary.mergedWith(shape.getPolygon().getBoundingBox());
    }
    // Build QuadTree
    spatial::QuadTree tree(boundary, 4, 8);
    for (const domain::Shape& shape : shapes) {
        if (shape.getLayer() != layer) {
            continue;
        }
        tree.insert(shape);
    }
    // Create query for each shape to get candidates
    // Then calculate distance between each pair
    for (const domain::Shape& shape : shapes) {
        if (shape.getLayer() != layer) {
            continue;
        }
        const geometry::BoundingBox searchRegion =shape.getPolygon().getBoundingBox().expanded(minimumSpacing);
        const auto candidates = tree.query(searchRegion);
        
        for (const domain::Shape* candidate :candidates)
        {
            // Avoid checking the shape against itself.
            if (candidate == &shape) {
                continue;
            }

            // Prevent duplicate A-B / B-A checks.
            // Shapes have unique IDs arranged ascendingly 
            if (candidate->getId() <= shape.getId())
            {
                continue;
            }

            const double actualSpacing = shape.getPolygon().distanceTo(candidate->getPolygon());

            if (actualSpacing + geometry::EPSILON <minimumSpacing)
            {
                violations.emplace_back(domain::ViolationType::MinSpacing, std::vector<std::size_t>{shape.getId(), candidate->getId()},
                    "Minimum spacing violation", actualSpacing, minimumSpacing);
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