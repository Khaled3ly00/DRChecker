#include "drcheck/spatial/LayerSpatialIndex.h"

#include <utility>

namespace drcheck::spatial {
    LayerSpatialIndex::LayerSpatialIndex(const std::vector<domain::Shape>& shapes, std::size_t capacity, std::size_t maxDepth)
    {
        // A map between each layer and containing shapes
        std::unordered_map<const domain::Layer*, std::vector<const domain::Shape*>> shapesByLayer;

        // fill map with layer and containing shapes
        for (const auto& shape : shapes)
        {
            // get layer of each shape and push shape into the map
            shapesByLayer[shape.getLayer()].push_back(&shape);
        }

        // loop through each layer to create QuadTree
        for (const auto& [layer, layerShapes] : shapesByLayer)
        {
            // initial root
            geometry::BoundingBox boundary = layerShapes[0]->getPolygon().getBoundingBox();
            // loop through each shape in this layer to calculate root bounding box
            for (std::size_t i = 1; i < layerShapes.size(); ++i)
            {
                boundary = boundary.mergedWith(layerShapes[i]->getPolygon().getBoundingBox());
            }
            // Create tree
            auto tree = std::make_unique<QuadTree>(boundary, capacity, maxDepth);
            // Insert this layer shapes in the tree
            for (const auto* shape : layerShapes)
            {
                tree->insert(*shape);
            }
            // add each Quadtree to relevant Layer in the trees map
            trees.emplace(layer, std::move(tree));
        }
    }

    // Does this layer have Quadtree created for it?
    bool LayerSpatialIndex::hasLayer(const domain::Layer* layer) const
    {
        return trees.find(layer) != trees.end();
    }

    std::vector<const domain::Shape*>LayerSpatialIndex::query(const domain::Layer* layer, const geometry::BoundingBox& region) const
    {
        // Check whether a QuadTree exists for this layer.
        const auto it = trees.find(layer);

        if (it == trees.end())
        {
            return {};
        }
        // Query the QuadTree associated with this layer.
        // it->second refers to value(tree) of key(layer)
        return it->second->query(region);
    }
}