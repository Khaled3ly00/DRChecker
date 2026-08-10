#include "drcheck/domain/Shape.h"

namespace drcheck::domain {
Shape::Shape(std::size_t id, Layer layer, geometry::Polygon polygon)
	: id(id), layer(layer), polygon(std::move(polygon)) {
}

std::size_t Shape::getId() const {
	return id;
}

Layer Shape::getLayer() const {
	return layer;
}

const geometry::Polygon& Shape::getPolygon() const {
	return polygon;
}
}