#pragma once

#include "Rule.h"
#include "drcheck/domain/Layer.h"

namespace drcheck::rules {
	// Represents a rule that checks for minimum width violations on a specific layer
class MinWidthRule : public Rule
{
public:
    MinWidthRule(domain::Layer layer, double minimumWidth);

    std::vector<domain::Violation> check (const std::vector<domain::Shape>& shapes, const spatial::LayerSpatialIndex& spatialIndex) const override;

    domain::Layer getLayer() const;

    double getMinimumWidth() const;

private:
    domain::Layer layer;
    double minimumWidth;
};

}