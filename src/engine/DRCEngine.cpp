#include "drcheck/engine/DRCEngine.h"
#include "drcheck/spatial/LayerSpatialIndex.h"

namespace drcheck::engine {

std::vector<domain::Violation> DRCEngine::run(const std::vector<domain::Shape>& shapes, const std::vector<std::unique_ptr<rules::Rule>>& rules) const
{
    std::vector<domain::Violation> violations;
    spatial::LayerSpatialIndex spatialIndex(shapes);

    for (const auto& rule : rules)
    {
        // Check if rule is a nullpointer
        if (!rule) {
            continue;
        }
        const auto ruleViolations = rule->check(shapes, spatialIndex);
        // Insert ruleViolations to the end of violations
        violations.insert(violations.end(), ruleViolations.begin(), ruleViolations.end());
    }
    return violations;
}

}