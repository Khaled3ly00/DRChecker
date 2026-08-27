#include <gtest/gtest.h>

#include "drcheck/domain/Shape.h"
#include "drcheck/geometry/Constants.h"

using drcheck::domain::Layer;
using drcheck::domain::Purpose;
using drcheck::domain::Shape;

using drcheck::geometry::Point;
using drcheck::geometry::Polygon;

using drcheck::geometry::EPSILON;

TEST(ShapeTest, StoresIdLayerAndPolygon)
{
    Polygon polygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 5),
        Point(0, 5)
        });

    Shape shape(
        7,
        Layer::M1,
        Purpose::Drawing,
        std::move(polygon)
    );

    EXPECT_EQ(shape.getId(), 7);
    EXPECT_EQ(shape.getLayer(), Layer::M1);
    EXPECT_EQ(shape.getPurpose(), Purpose::Drawing);
    EXPECT_NEAR(shape.getPolygon().area(), 50.0, EPSILON);
}

