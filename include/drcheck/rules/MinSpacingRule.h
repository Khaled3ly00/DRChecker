#pragma once

#include "Rule.h"
#include "drcheck/domain/Layer.h"

namespace drcheck::rules {
    // Represents a rule that checks for minimum spacing violations on a specific layer
class MinSpacingRule : public Rule
{
public:
    MinSpacingRule(const domain::Layer* layer, double minimumSpacing);

    std::vector<domain::Violation> check(const std::vector<domain::Shape>& shapes, const spatial::LayerSpatialIndex& spatialIndex) const override;

    const domain::Layer* getLayer() const;

    double getMinimumSpacing() const;

private:
    const domain::Layer* layer;
    double minimumSpacing;
};

}