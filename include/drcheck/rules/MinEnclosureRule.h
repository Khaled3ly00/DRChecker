#pragma once

#include "Rule.h"
#include "drcheck/domain/Layer.h"

namespace drcheck::rules {
class MinEnclosureRule : public Rule
{
public:
    MinEnclosureRule(domain::Layer innerLayer, domain::Layer outerLayer, double minimumEnclosure);

    std::vector<domain::Violation> check(const std::vector<domain::Shape>& shapes, const spatial::LayerSpatialIndex& spatialIndex) const override;

    domain::Layer getInnerLayer() const;
    domain::Layer getOuterLayer() const;
    double getMinimumEnclosure() const;

private:
    domain::Layer innerLayer;
    domain::Layer outerLayer;
    double minimumEnclosure;
};
}