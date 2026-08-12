#include "drcheck/engine/DRCEngine.h"

namespace drcheck::engine {

/**
    * @brief Execute the rule-checking engine.
    *
    * Runs each rule in `rules` against the provided `shapes` collection and
    * aggregates all resulting violations into a single vector which is returned.
    *
    * @param shapes Read-only collection of shapes to validate.
    * @param rules Read-only collection of rule instances (owned by unique_ptr).
    * @return std::vector<domain::Violation> Aggregated violations from all rules.
    *
    * Remarks:
    * - This method is const-qualified and does not modify the input containers.
    * - Complexity depends on the number of rules and the cost of each rule's `check` implementation.
    * - Each rule's `check` is expected to return its violations; this method simply concatenates them.
    */
std::vector<domain::Violation> DRCEngine::run(const std::vector<domain::Shape>& shapes, const std::vector<std::unique_ptr<rules::Rule>>& rules) const
{
    std::vector<domain::Violation> violations;

    for (const auto& rule : rules)
    {
        // Check if rule is a nullpointer
        if (!rule) {
            continue;
        }
        const auto ruleViolations = rule->check(shapes);
        // Insert ruleViolations to the end of violations
        violations.insert(violations.end(), ruleViolations.begin(), ruleViolations.end());
    }
    return violations;
}

}