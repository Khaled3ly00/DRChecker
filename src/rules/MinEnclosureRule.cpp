#include "drcheck/rules/MinEnclosureRule.h"
#include "drcheck/geometry/Constants.h"

#include <stdexcept>

namespace drcheck::rules {

MinEnclosureRule::MinEnclosureRule(domain::Layer innerLayer, domain::Layer outerLayer, double minimumEnclosure)
    : innerLayer(innerLayer), outerLayer(outerLayer), minimumEnclosure(minimumEnclosure)
{
    if (minimumEnclosure <= 0.0) {
        throw std::invalid_argument(
            "Minimum enclosure must be positive"
        );
    }
    if (innerLayer == outerLayer) {
        throw std::invalid_argument(
            "Enclosure rule requires different "
            "inner and outer layers"
        );
    }
}

std::vector<domain::Violation> MinEnclosureRule::check(const std::vector<domain::Shape>& shapes) const
{
    std::vector<domain::Violation> violations;
    // Check every shape if it has a layer that matches inner layer
    for (const domain::Shape& inner : shapes)
    {
        if (inner.getLayer() != innerLayer) {
            continue;
        }
        // If shape has inner layer matching check it against outerlayer enclosure
        bool properlyEnclosed = false;
        for (const domain::Shape& outer : shapes)
        {
            if (outer.getLayer() != outerLayer) {
                continue;
            }
            // Check for strict containment (no intersection between edges) note: touching not considered intersection
            if (!outer.getPolygon().contains(inner.getPolygon()))
            {
                continue;
            }
            // Calculate distance between shapes (if touching distance = 0)
            const double actualEnclosure = outer.getPolygon().distanceTo(inner.getPolygon(), false);

            if (actualEnclosure + geometry::EPSILON >= minimumEnclosure)
            {
                properlyEnclosed = true;
                break;
            }
        }

        if (!properlyEnclosed)
        {
            violations.emplace_back(domain::ViolationType::Enclosure, std::vector<std::size_t>{inner.getId()}, "Enclosure violation" );
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