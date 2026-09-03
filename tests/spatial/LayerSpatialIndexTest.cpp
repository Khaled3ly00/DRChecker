#include <gtest/gtest.h>

#include "drcheck/spatial/LayerSpatialIndex.h"
#include "drcheck/domain/LayerRegistry.h"

using drcheck::spatial::LayerSpatialIndex;
using drcheck::spatial::QuadTree;
using drcheck::geometry::BoundingBox;
using drcheck::geometry::Polygon;
using drcheck::geometry::Point;
using drcheck::domain::Shape;
using drcheck::domain::LayerRegistry;

TEST(LayerSpatialIndexTest, EmptyLayoutContainsNoSpatialIndexForLayer)
{
	// Create a LayerRegistry and declare some layers (tech layers)
    LayerRegistry registry;
    registry.declare("M1");
    registry.declare("M2");
    registry.declare("VIA1");

	// Empty vector of shapes, so no spatial index will be built for any layer
    const std::vector<Shape> shapes;
    LayerSpatialIndex index(shapes);

    EXPECT_FALSE(index.hasLayer(registry.resolve("M1")));
    EXPECT_FALSE(index.hasLayer(registry.resolve("M2")));
    EXPECT_FALSE(index.hasLayer(registry.resolve("VIA1")));
}

TEST(LayerSpatialIndexTest, BuildsIndexForExistingLayer)
{
    LayerRegistry registry;
    const auto* M1 = registry.declare("M1");

    Polygon polygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });
    Shape shape(1, M1, std::move(polygon));
    const std::vector<Shape> shapes{std::move(shape)};
    LayerSpatialIndex index(shapes);

    EXPECT_TRUE(index.hasLayer(M1));
}

TEST(LayerSpatialIndexTest, DoesNotBuildIndexForMissingLayer)
{
    LayerRegistry registry;
    const auto* M1 = registry.declare("M1");
    const auto* M2 = registry.declare("M2");
    const auto* VIA1 = registry.declare("VIA1");

    Polygon polygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });
    Shape shape(1, M1, std::move(polygon));
    const std::vector<Shape> shapes{std::move(shape)};
    LayerSpatialIndex index(shapes);

    EXPECT_TRUE(index.hasLayer(M1));
    EXPECT_FALSE(index.hasLayer(M2));
    EXPECT_FALSE(index.hasLayer(VIA1));
}

TEST(LayerSpatialIndexTest, QueryReturnsNearbyShapeOnRequestedLayer)
{
    LayerRegistry registry;
    const auto* M1 = registry.declare("M1");

    Polygon firstPolygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });
    Polygon secondPolygon({
        Point(100, 100),
        Point(110, 100),
        Point(110, 110),
        Point(100, 110)
        });
    Shape first(1, M1, std::move(firstPolygon));
    Shape second(2, M1, std::move(secondPolygon));
    const std::vector<Shape> shapes{std::move(first),std::move(second)};
    LayerSpatialIndex index(shapes);
    const BoundingBox searchRegion(-5, -5, 15, 15);
    const auto results = index.query(M1, searchRegion);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->getId(), 1);
}

TEST(LayerSpatialIndexTest, QueryDoesNotReturnShapeFromDifferentLayer)
{
    LayerRegistry registry;
    const auto* M1 = registry.declare("M1");
    const auto* M2 = registry.declare("M2");

    Polygon firstPolygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });
    Polygon secondPolygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });
    Shape first(1, M1, std::move(firstPolygon));
    Shape second(2, M2, std::move(secondPolygon));
    const std::vector<Shape> shapes{std::move(first), std::move(second)};
    LayerSpatialIndex index(shapes);
    const BoundingBox searchRegion(-5, -5, 15, 15);
    const auto results = index.query(M1, searchRegion);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->getId(), 1);
    EXPECT_EQ(results[0]->getLayer(), M1);
}

TEST(LayerSpatialIndexTest, QueryMissingLayerReturnsEmptyResult)
{
    LayerRegistry registry;
    const auto* M1 = registry.declare("M1");
    const auto* M2 = registry.declare("M2");

    Polygon polygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });
    Shape shape(1, M1, std::move(polygon));
    const std::vector<Shape> shapes{std::move(shape)};
    LayerSpatialIndex index(shapes);
    const BoundingBox searchRegion(-100, -100, 100, 100);
    const auto results = index.query(M2, searchRegion);

    EXPECT_TRUE(results.empty());
}
