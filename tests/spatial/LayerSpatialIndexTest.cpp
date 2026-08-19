#include <gtest/gtest.h>

#include "drcheck/spatial/LayerSpatialIndex.h"

using drcheck::spatial::LayerSpatialIndex;
using drcheck::spatial::QuadTree;
using drcheck::geometry::BoundingBox;
using drcheck::geometry::Polygon;
using drcheck::geometry::Point;
using drcheck::domain::Shape;
using drcheck::domain::Layer;

TEST(LayerSpatialIndexTest, EmptyLayoutContainsNoLayer)
{
    const std::vector<Shape> shapes;
    LayerSpatialIndex index(shapes);

    EXPECT_FALSE(index.hasLayer(Layer::Metal1));
    EXPECT_FALSE(index.hasLayer(Layer::Metal2));
    EXPECT_FALSE(index.hasLayer(Layer::Via12));
}

TEST(LayerSpatialIndexTest, BuildsIndexForExistingLayer)
{
    Polygon polygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });
    Shape shape(1, Layer::Metal1, std::move(polygon));
    const std::vector<Shape> shapes{std::move(shape)};
    LayerSpatialIndex index(shapes);

    EXPECT_TRUE(index.hasLayer(Layer::Metal1));
}

TEST(LayerSpatialIndexTest, DoesNotBuildIndexForMissingLayer)
{
    Polygon polygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });
    Shape shape(1, Layer::Metal1, std::move(polygon));
    const std::vector<Shape> shapes{std::move(shape)};
    LayerSpatialIndex index(shapes);

    EXPECT_TRUE(index.hasLayer(Layer::Metal1));
    EXPECT_FALSE(index.hasLayer(Layer::Metal2));
    EXPECT_FALSE(index.hasLayer(Layer::Via12));
}

TEST(LayerSpatialIndexTest, QueryReturnsNearbyShapeOnRequestedLayer)
{
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
    Shape first(1, Layer::Metal1, std::move(firstPolygon));
    Shape second(2, Layer::Metal1, std::move(secondPolygon));
    const std::vector<Shape> shapes{std::move(first),std::move(second)};
    LayerSpatialIndex index(shapes);
    const BoundingBox searchRegion(-5, -5, 15, 15);
    const auto results = index.query(Layer::Metal1, searchRegion);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->getId(), 1);
}

TEST(LayerSpatialIndexTest, QueryDoesNotReturnShapeFromDifferentLayer)
{
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
    Shape first(1, Layer::Metal1, std::move(firstPolygon));
    Shape second(2, Layer::Metal2,std::move(secondPolygon));
    const std::vector<Shape> shapes{std::move(first), std::move(second)};
    LayerSpatialIndex index(shapes);
    const BoundingBox searchRegion(-5, -5, 15, 15);
    const auto results = index.query(Layer::Metal1, searchRegion);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->getId(), 1);
    EXPECT_EQ(results[0]->getLayer(), Layer::Metal1);
}

TEST(LayerSpatialIndexTest, QueryMissingLayerReturnsEmptyResult)
{
    Polygon polygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });
    Shape shape(1, Layer::Metal1, std::move(polygon));
    const std::vector<Shape> shapes{std::move(shape)};
    LayerSpatialIndex index(shapes);
    const BoundingBox searchRegion(-100, -100, 100, 100);
    const auto results = index.query(Layer::Metal2, searchRegion);

    EXPECT_TRUE(results.empty());
}