#include "drcheck/rules/MinEnclosureRule.h"
#include "drcheck/geometry/Constants.h"

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

std::vector<domain::Violation>MinEnclosureRule::check(const std::vector<domain::Shape>& shapes) const
{
    std::vector<domain::Violation> violations;

    for (const domain::Shape& inner : shapes)
    {
        if (inner.getLayer() != innerLayer) {
            continue;
        }

        bool foundContainingOuter = false;

        for (const domain::Shape& outer : shapes)
        {
            if (outer.getLayer() != outerLayer) {
                continue;
            }

            if (!outer.getPolygon().contains(inner.getPolygon()))
            {
                continue;
            }

            foundContainingOuter = true;

            const double actualEnclosure =
                outer.getPolygon().distanceTo(inner.getPolygon(), false);

            if (actualEnclosure + geometry::EPSILON <
                minimumEnclosure)
            {
                violations.emplace_back(
                    domain::ViolationType::Enclosure, std::vector<std::size_t>{inner.getId(), outer.getId()}, "Enclosure violation", actualEnclosure, minimumEnclosure);
            }

            // Current assumption:
            // only one outer shape will contain this inner shape.
            break;
        }

        if (!foundContainingOuter)
        {
            violations.emplace_back(domain::ViolationType::Enclosure, std::vector<std::size_t>{ inner.getId()}, "Inner shape is not enclosed by any outer shape", 0.0, minimumEnclosure);
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