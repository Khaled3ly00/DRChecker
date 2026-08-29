#include "drcheck/rules/MinEnclosureRule.h"
#include "drcheck/geometry/Constants.h"
#include "drcheck/spatial/LayerSpatialIndex.h"

#include <stdexcept>

namespace drcheck::rules {

MinEnclosureRule::MinEnclosureRule(domain::Layer innerLayer, domain::Layer outerLayer, double minimumEnclosure)
    : innerLayer(innerLayer), outerLayer(outerLayer), minimumEnclosure(minimumEnclosure)
{
    if (minimumEnclosure <= 0.0) {
        throw std::invalid_argument("Minimum enclosure must be positive");
    }
    if (innerLayer == outerLayer) {
        throw std::invalid_argument(
            "Enclosure rule requires different "
            "inner and outer layers"
        );
    }
}

std::vector<domain::Violation>MinEnclosureRule::check(const std::vector<domain::Shape>& shapes, const spatial::LayerSpatialIndex& spatialIndex) const
{
    std::vector<domain::Violation> violations;

    for (const domain::Shape& innerShape : shapes)
    {
        if (innerShape.getLayer() != innerLayer) {
            continue;
        }

        bool foundContainingOuter = false;

        const auto innerBox = innerShape.getPolygon().getBoundingBox();
        // query() return any outerLayer shapes that have shared area with innerBox 
        const auto candidates = spatialIndex.query(outerLayer, innerBox);

        for (const domain::Shape* outerShape : candidates)
        {
            const auto& innerPolygon = innerShape.getPolygon();
            const auto& outerPolygon = outerShape->getPolygon();

            if (!outerPolygon.contains(innerPolygon))
            {
                continue;
            }

            foundContainingOuter = true;

            const geometry::PolygonEdgePairResult actualEnclosure = innerPolygon.distanceTo(outerPolygon, false);

            if (actualEnclosure.distance + geometry::EPSILON < minimumEnclosure)
            {
                domain::ViolationMarker marker{
                    .firstPoint = actualEnclosure.firstPoint,
                    .secondPoint = actualEnclosure.secondPoint,
                    .firstEdgeIndex = actualEnclosure.firstEdgeIndex,
                    .secondEdgeIndex = actualEnclosure.secondEdgeIndex,
                    .firstLayer = getInnerLayer(),
                    .secondLayer = getOuterLayer()
                };

                violations.emplace_back(
                    domain::ViolationType::Enclosure, std::vector<std::size_t>{innerShape.getId(), outerShape->getId()}, "Enclosure violation", actualEnclosure.distance, minimumEnclosure, marker);
            }

            // Current assumption:
            // only one outer shape will contain this inner shape.
            break;
        }

        if (!foundContainingOuter)
        {
            violations.emplace_back(domain::ViolationType::Enclosure, std::vector<std::size_t>{ innerShape.getId()}, "Inner shape is not enclosed by any outer shape", 0.0, minimumEnclosure);
        }
    }

    return violations;
}

domain::Layer MinEnclosureRule::getInnerLayer() const
{
    return innerLayer;
}

domain::Layer MinEnclosureRule::getOuterLayer() const
{
    return outerLayer;
}

double MinEnclosureRule::getMinimumEnclosure() const
{
    return minimumEnclosure;
}

}