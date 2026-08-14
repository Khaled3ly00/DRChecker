#include "drcheck/rules/MinSpacingRule.h"
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
    // Loop through all pairs of shapes to check for minimum spacing violations
    for (std::size_t i = 0; i < shapes.size(); ++i)
    {
        for (std::size_t j = i + 1; j < shapes.size(); ++j)
        {
            const domain::Shape& first = shapes[i];

            const domain::Shape& second = shapes[j];
            // Make sure both shapes are on the same layer as the rule
            if (first.getLayer() != layer || second.getLayer() != layer)
            {
                continue;
            }

            const double actualSpacing = first.getPolygon().distanceTo(second.getPolygon());

            if (actualSpacing + geometry::EPSILON >= minimumSpacing)
            {
                continue;
            }

            violations.emplace_back(domain::ViolationType::MinSpacing, std::vector<std::size_t>{first.getId(), second.getId()}, "Minimum spacing violation", actualSpacing, minimumSpacing);
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