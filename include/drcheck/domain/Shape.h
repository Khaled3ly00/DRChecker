#pragma once

#include <cstddef>

#include "drcheck/geometry/Polygon.h"
#include "Layer.h"

namespace drcheck::domain {
class Shape
{
public:
    Shape(std::size_t id, Layer layer, geometry::Polygon polygon);

    std::size_t getId() const;

    Layer getLayer() const;

    const geometry::Polygon& getPolygon() const;

private:
    std::size_t id;
    Layer layer;
    geometry::Polygon polygon;
};
}