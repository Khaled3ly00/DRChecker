#include <gtest/gtest.h>

#include "drcheck/domain/Shape.h"
#include "drcheck/domain/Layer.h"
#include "drcheck/domain/LayerRegistry.h"
#include "drcheck/geometry/Polygon.h"
#include "drcheck/geometry/Point.h"
#include "drcheck/geometry/Constants.h"

using drcheck::domain::Layer;
using drcheck::domain::Shape;
using drcheck::domain::LayerRegistry;

using drcheck::geometry::Point;
using drcheck::geometry::Polygon;

using drcheck::geometry::EPSILON;

TEST(ShapeTest, StoresIdLayerAndPolygon)
{
    LayerRegistry registry;
    const Layer* m1 = registry.declare("M1");

    Polygon polygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 5),
        Point(0, 5)
        });

    Shape shape(7, m1, std::move(polygon));

    EXPECT_EQ(shape.getId(), 7);
    EXPECT_EQ(shape.getLayer(), m1);
    EXPECT_NEAR(shape.getPolygon().area(), 50.0, EPSILON);
}

TEST(ShapeTest, ThrowsOnNullPointerLayer)
{
    Polygon polygon({
    Point(0, 0),
    Point(10, 0),
    Point(10, 5),
    Point(0, 5)
        });

    EXPECT_THROW(Shape(1, nullptr, std::move(polygon)), std::invalid_argument);
}