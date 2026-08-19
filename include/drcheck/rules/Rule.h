#pragma once

#include <vector>

#include "drcheck/domain/Shape.h"
#include "drcheck/domain/Violation.h"
#include "drcheck/spatial/LayerSpatialIndex.h"

using drcheck::spatial::LayerSpatialIndex;

namespace drcheck::rules {

class Rule
{
public:
    virtual ~Rule() = default;

    virtual std::vector<domain::Violation> check(const std::vector<domain::Shape>& shapes, const LayerSpatialIndex& spatialIndex) const = 0;
};

}