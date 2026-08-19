#pragma once

#include "drcheck/domain/Layer.h"
#include "drcheck/domain/Shape.h"
#include "drcheck/geometry/BoundingBox.h"
#include "drcheck/spatial/QuadTree.h"

#include <cstddef>
#include <map>
#include <memory>
#include <vector>

namespace drcheck::spatial {

    class LayerSpatialIndex
    {
    public:
        LayerSpatialIndex(const std::vector<domain::Shape>& shapes, std::size_t capacity = 16,std::size_t maxDepth = 8);

        std::vector<const domain::Shape*> query(domain::Layer layer, const geometry::BoundingBox& region) const;

        bool hasLayer(domain::Layer layer) const;

    private:
        // map between each layer and pointer to QuadTree that's not created yet
        std::map<domain::Layer,std::unique_ptr<QuadTree>> trees;
    };

}