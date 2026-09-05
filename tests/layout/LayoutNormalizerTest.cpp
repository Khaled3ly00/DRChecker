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
TEST(LayoutNormalizerTest, SpatialQueryBoundingBoxTolerance)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    Polygon first({
        Point(-0.065000000000000002, 0.54000000000000004),
        Point(0.084999999999999992, 0.54000000000000004),
        Point(0.084999999999999992, 0.69000000000000006),
        Point(-0.065000000000000002, 0.69000000000000006)
        });

    Polygon second({
        Point(0.24500000000000000, 0.55500000000000005),
        Point(0.34499999999999997, 0.55500000000000005),
        Point(0.34499999999999997, 0.67500000000000004),
        Point(0.24500000000000000, 0.67500000000000004)
        });

    Polygon third({
        Point(0.18500000000000000, 0.55500000000000005),
        Point(0.24500000000000000, 0.55500000000000005),
        Point(0.24500000000000000, 0.67500000000000004),
        Point(0.18500000000000000, 0.67500000000000004)
        });

    Polygon fourth({
        Point(-0.065000000000000002, 0.55500000000000005),
        Point(0.18500000000000000, 0.55500000000000005),
        Point(0.18500000000000000, 0.67500000000000004),
        Point(-0.065000000000000002, 0.67500000000000004)
        });

    Polygon fifth({
        Point(0.90500000000000003, 0.54000000000000004),
        Point(0.75500000000000000, 0.54000000000000004),
        Point(0.75500000000000000, 0.69000000000000006),
        Point(0.90500000000000003, 0.69000000000000006)
        });

    Polygon six({
        Point(0.49500000000000000, 0.54000000000000004),
        Point(0.34500000000000003, 0.54000000000000004),
        Point(0.34500000000000003, 0.69000000000000006),
        Point(0.49500000000000005, 0.69000000000000006)
        });

    Polygon seven({
        Point(0.59499999999999997, 0.55500000000000005),
        Point(0.34500000000000003, 0.55500000000000005),
        Point(0.34500000000000003, 0.67500000000000004),
        Point(0.59500000000000008, 0.67500000000000004)
        });

    Polygon eight({
        Point(0.65500000000000003, 0.55500000000000005),
        Point(0.59499999999999997, 0.55500000000000005),
        Point(0.59500000000000008, 0.67500000000000004),
        Point(0.65500000000000003, 0.67500000000000004)
        });

    Polygon nine({
        Point(0.90500000000000003, 0.55500000000000005),
        Point(0.65500000000000003, 0.55500000000000005),
        Point(0.65500000000000003, 0.67500000000000004),
        Point(0.90500000000000003, 0.67500000000000004)
        });

    Shape shape1(1, M1, std::move(first));
    Shape shape2(2, M1, std::move(second));
    Shape shape3(3, M1, std::move(third));
    Shape shape4(4, M1, std::move(fourth));
    Shape shape5(5, M1, std::move(fifth));
    Shape shape6(6, M1, std::move(six));
    Shape shape7(7, M1, std::move(seven));
    Shape shape8(8, M1, std::move(eight));
    Shape shape9(9, M1, std::move(nine));

    const std::vector<Shape> shapes{ shape1, shape2, shape3, shape4, shape5, shape6, shape7, shape8, shape9 };
    const auto normalized = LayoutNormalizer::normalize(shapes);

    ASSERT_EQ(normalized.size(), 1);
}