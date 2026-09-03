#pragma once

#include <vector>
#include <unordered_set>

#include "drcheck/domain/Shape.h"
#include "drcheck/spatial/LayerSpatialIndex.h"

namespace drcheck::layout {

class LayoutNormalizer
{
public:
    static std::vector<domain::Shape> normalize(const std::vector<domain::Shape>& shapes);
    static std::vector<const domain::Shape*> collectConnectedComponent(const domain::Shape& start, const spatial::LayerSpatialIndex& spatialIndex, std::unordered_set<const domain::Shape*>& visited);

private:
    static bool areMergeable(const domain::Shape& first, const domain::Shape& second);
    static domain::Shape mergeComponent(std::size_t normalizedId, const std::vector<const domain::Shape*>& component);
};

}