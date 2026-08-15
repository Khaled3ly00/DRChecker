#pragma once

#include "drcheck/geometry/BoundingBox.h"
#include "drcheck/domain/Shape.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace drcheck::spatial {

    class QuadTree
    {
    public:
        QuadTree(geometry::BoundingBox boundary, std::size_t capacity, std::size_t maxDepth);

        void insert(const domain::Shape& shape);

        std::vector<const domain::Shape*> query(const geometry::BoundingBox& region) const;
    private:
        QuadTree(geometry::BoundingBox boundary, std::size_t capacity, std::size_t maxDepth, std::size_t depth);

        void subdivide();

        bool isSubdivided() const;

        QuadTree* getContainingChild(const geometry::BoundingBox& box);

        void queryRecursive(const geometry::BoundingBox& region, std::vector<const domain::Shape*>& results) const;

        geometry::BoundingBox boundary;
        std::size_t capacity;
        std::size_t maxDepth;
        std::size_t depth;
        
        std::unique_ptr<QuadTree> northWest;
        std::unique_ptr<QuadTree> northEast;
        std::unique_ptr<QuadTree> southWest;
        std::unique_ptr<QuadTree> southEast;

        std::vector<const domain::Shape*> shapes;
    };

}