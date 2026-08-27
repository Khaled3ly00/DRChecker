#include "drcheck/domain/Shape.h"

namespace drcheck::domain {
Shape::Shape(std::size_t id, Layer layer, Purpose purpose, geometry::Polygon polygon)
	: id(id), layer(layer), purpose(purpose), polygon(std::move(polygon)) {
}

std::size_t Shape::getId() const {
	return id;
}

Layer Shape::getLayer() const {
	return layer;
}

Purpose Shape::getPurpose() const {
	return purpose;
}

const geometry::Polygon& Shape::getPolygon() const {
	return polygon;
}
}