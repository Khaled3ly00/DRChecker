#pragma once

#include <cstddef>

#include "drcheck/geometry/Polygon.h"
#include "drcheck/domain/Layer.h"

namespace drcheck::domain {

class Shape
{
public:
    Shape(std::size_t id, const Layer* layer, geometry::Polygon polygon);

    std::size_t getId() const;

    const Layer* getLayer() const;

    const geometry::Polygon& getPolygon() const;

private:
    std::size_t id;
    const Layer* layer;
    geometry::Polygon polygon;
};
}
