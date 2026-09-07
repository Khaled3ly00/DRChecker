#include "drcheck/domain/Shape.h"

#include <stdexcept>

namespace drcheck::domain {
Shape::Shape(std::size_t id, const Layer* layer, geometry::Polygon polygon)
	: id(id), layer(layer), polygon(std::move(polygon)) {
	if (layer == nullptr)
	{
		throw std::invalid_argument("Shape layer cannot be null");
	}
	if (!getPolygon().isOctilinear()) {
		throw std::invalid_argument("DRCheck supports only orthogonal and 45-degree polygons");
	}
}

std::size_t Shape::getId() const
{
	return id;
}

const Layer* Shape::getLayer() const {
	return layer;
}

const geometry::Polygon& Shape::getPolygon() const {
	return polygon;
}
}
