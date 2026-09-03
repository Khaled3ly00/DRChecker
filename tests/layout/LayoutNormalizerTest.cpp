#include <gtest/gtest.h>

#include "drcheck/layout/LayoutNormalizer.h"
#include "drcheck/geometry/Constants.h"
#include "drcheck/domain/LayerRegistry.h"

#include <algorithm>
#include <stdexcept>

using namespace drcheck::layout;
using drcheck::geometry::Polygon;
using drcheck::geometry::Point;
using drcheck::geometry::EPSILON;
using drcheck::domain::Shape;
using drcheck::domain::LayerRegistry;
using drcheck::domain::Layer;

TEST(LayoutNormalizerTest, OverlappingPolygonsWithSameLayerMergable)
{

    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* M2 = registry.declare("M2");

    Polygon first({
    Point(0, 0),
    Point(4, 0),
    Point(4, 4),
    Point(0, 4)
        });

    Polygon second({
        Point(2, 6),
        Point(3, 6),
        Point(3, 2),
        Point(2, 2)
        });

    Shape shape1(1, M1, std::move(first));
    Shape shape2(2, M1, std::move(second));
    const std::vector<Shape> shapes{ shape1, shape2 };

    const auto normalized = LayoutNormalizer::normalize(shapes);

    ASSERT_EQ(normalized.size(), 1);
}

TEST(LayoutNormalizerTest, SharingFullBoundaryWithSameLayerMergable)
{

    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* M2 = registry.declare("M2");

    Polygon first({
        Point(0, 0),
        Point(4, 0),
        Point(4, 4),
        Point(0, 4)
        });

    Polygon second({
        Point(4, 0),
        Point(10, 0),
        Point(10, 4),
        Point(4, 4)
        });

    Shape shape1(1, M1, std::move(first));
    Shape shape2(2, M1, std::move(second));
    const std::vector<Shape> shapes{ shape1, shape2 };

    const auto normalized = LayoutNormalizer::normalize(shapes);

    const auto& vertices =normalized[0].getPolygon().getVertices();
    ASSERT_EQ(vertices.size(), 6);

    const auto containsVertex = [&vertices](double x, double y)
        {
            return std::any_of(vertices.begin(), vertices.end(), [x, y](const Point& point)
                {
                    return point.isNear(Point(x, y));
                }
            );
        };

    EXPECT_TRUE(containsVertex(0, 0));
    EXPECT_TRUE(containsVertex(10, 0));
    EXPECT_TRUE(containsVertex(10, 4));
    EXPECT_TRUE(containsVertex(0, 4));
    EXPECT_TRUE(containsVertex(4, 0));
    EXPECT_TRUE(containsVertex(4, 4));

    EXPECT_NEAR(normalized[0].getPolygon().area(), 40.0, EPSILON);

}


TEST(LayoutNormalizerTest, SharingPartialBoundaryWithSameLayerMergable)
{

    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* M2 = registry.declare("M2");

    Polygon first({
        Point(0, 0),
        Point(4, 0),
        Point(4, 4),
        Point(0, 4)
        });

    Polygon second({
        Point(4, 0),
        Point(10, 0),
        Point(10, 2),
        Point(4, 2)
        });

    Shape shape1(1, M1, std::move(first));
    Shape shape2(2, M1, std::move(second));
    const std::vector<Shape> shapes{ shape1, shape2 };

    const auto normalized = LayoutNormalizer::normalize(shapes);

    ASSERT_EQ(normalized.size(), 1);
}

TEST(LayoutNormalizerTest, TouchingVerticesPolygonsWithSameLayerNotMergable)
{

    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* M2 = registry.declare("M2");

    Polygon first({
        Point(0, 0),
        Point(4, 0),
        Point(4, 4),
        Point(0, 4)
        });

    Polygon second({
        Point(4, 4),
        Point(10, 4),
        Point(10, 8),
        Point(4, 8)
        });

    Shape shape1(1, M1, std::move(first));
    Shape shape2(2, M1, std::move(second));
    const std::vector<Shape> shapes{ shape1, shape2 };

    const auto normalized = LayoutNormalizer::normalize(shapes);

    ASSERT_EQ(normalized.size(), 2);
}

TEST(LayoutNormalizerTest, SeparatedPolygonsWithSameLayerNotMergable)
{

    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* M2 = registry.declare("M2");

    Polygon first({
        Point(0.0, 0.0),
        Point(4.0, 0.0),
        Point(4.0, 4.0),
        Point(0.0, 4.0)
        });

    Polygon second({
        Point(10.0, 0.0),
        Point(14.0, 0.0),
        Point(14.0, 4.0),
        Point(10.0, 4.0)
        });

    Shape shape1(1, M1, std::move(first));
    Shape shape2(2, M1, std::move(second));
    const std::vector<Shape> shapes{ shape1, shape2 };

    const auto normalized = LayoutNormalizer::normalize(shapes);

    ASSERT_EQ(normalized.size(), 2);
}

TEST(LayoutNormalizerTest, OverlappingPolygonsWithDifferentLayerNotMergable)
{

    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* M2 = registry.declare("M2");

    Polygon first({
    Point(0, 0),
    Point(4, 0),
    Point(4, 4),
    Point(0, 4)
        });

    Polygon second({
        Point(2, 6),
        Point(3, 6),
        Point(3, 2),
        Point(2, 2)
        });

    Shape shape1(1, M1, std::move(first));
    Shape shape2(2, M2, std::move(second));
    const std::vector<Shape> shapes{ shape1, shape2 };

    const auto normalized = LayoutNormalizer::normalize(shapes);

    ASSERT_EQ(normalized.size(), 2);
}

TEST(LayoutNormalizerTest, ThreeConsecutivePolygonsWithSameLayerMergable)
{

    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* M2 = registry.declare("M2");

    Polygon first({
        Point(0, 0),
        Point(4, 0),
        Point(4, 4),
        Point(0, 4)
        });

    Polygon second({
        Point(4, 0),
        Point(10, 0),
        Point(10, 4),
        Point(4, 4)
        });

    Polygon third({
    Point(10, 0),
    Point(14, 0),
    Point(14, 4),
    Point(10, 4)
        });

    Shape shape1(1, M1, std::move(first));
    Shape shape2(2, M1, std::move(second));
    Shape shape3(3, M1, std::move(third));
    const std::vector<Shape> shapes{ shape1, shape2, shape3 };

    const auto normalized = LayoutNormalizer::normalize(shapes);

    ASSERT_EQ(normalized.size(), 1);

    EXPECT_EQ(normalized[0].getId(), 1);

    const auto& vertices = normalized[0].getPolygon().getVertices();
    ASSERT_EQ(vertices.size(), 8);

    EXPECT_NEAR(normalized[0].getPolygon().area(), 56.0, EPSILON);

}
