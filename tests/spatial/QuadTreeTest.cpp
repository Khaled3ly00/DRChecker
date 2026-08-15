#include <gtest/gtest.h>
#include <stdexcept>

#include "drcheck/spatial/QuadTree.h"

using drcheck::spatial::QuadTree;
using drcheck::geometry::BoundingBox;
using drcheck::geometry::Polygon;
using drcheck::geometry::Point;
using drcheck::domain::Shape;
using drcheck::domain::Layer;

TEST(QuadTreeTest, RejectsZeroCapacity)
{
    BoundingBox boundary(0, 0, 100, 100);

    EXPECT_THROW(QuadTree(boundary,0,4), std::invalid_argument);
}

TEST(QuadTreeTest, InsertOneShapeThenQueryRelatedRegion)
{
    BoundingBox boundary(0, 0, 100, 100);

    Polygon polygon({
        Point(10, 10),
        Point(20, 10),
        Point(20, 20),
        Point(10, 20)
        });
    Shape shape(1, Layer::Metal1, std::move(polygon));

    QuadTree quadtree(boundary, 2, 5);

    BoundingBox region(0, 0, 30, 30);

    quadtree.insert(shape);

    std::vector<const Shape*> results;
    results = quadtree.query(region);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->getId(), 1);
}

TEST(QuadTreeTest, InsertOneShapeThenQueryUnrelatedRegion)
{
    BoundingBox boundary(0, 0, 100, 100);

    Polygon polygon({
        Point(10, 10),
        Point(20, 10),
        Point(20, 20),
        Point(10, 20)
        });
    Shape shape(1, Layer::Metal1, std::move(polygon));

    QuadTree quadtree(boundary, 2, 5);

    BoundingBox region(70, 70, 90, 90);

    quadtree.insert(shape);

    std::vector<const Shape*> results;
    results = quadtree.query(region);

    EXPECT_TRUE(results.empty());
}

TEST(QuadTreeTest, InsertMultipleShapesThenQuery)
{
    BoundingBox boundary(0, 0, 100, 100);

    Polygon polygon1({
        Point(10, 10),
        Point(20, 10),
        Point(20, 20),
        Point(10, 20)
        });
    Shape shape1(1, Layer::Metal1, std::move(polygon1));

    Polygon polygon2({
        Point(60, 60),
        Point(80, 60),
        Point(80, 80),
        Point(60, 80)
        });
    Shape shape2(2, Layer::Metal1, std::move(polygon2));

    Polygon polygon3({
        Point(30, 60),
        Point(40, 60),
        Point(40, 80),
        Point(30, 80)
        });
    Shape shape3(3, Layer::Metal1, std::move(polygon3));

    Polygon polygon4({
        Point(70, 60),
        Point(85, 60),
        Point(85, 80),
        Point(70, 80)
        });
    Shape shape4(4, Layer::Metal1, std::move(polygon4));

    QuadTree quadtree(boundary, 1, 5);

    const std::vector<Shape> shapes {shape1, shape2, shape3, shape4};
    for (const Shape& shape : shapes) {
        quadtree.insert(shape);
    }

    BoundingBox region1(0, 50, 50, 100);
    BoundingBox region2(0, 0, 100, 100);

    const auto results1 = quadtree.query(region1);
    const auto results2 = quadtree.query(region2);

    ASSERT_EQ(results1.size(), 1);
    EXPECT_EQ(results1[0]->getId(), 3);
    ASSERT_EQ(results2.size(), 4);
}

TEST(QuadTreeTest, InsertBoundaryCrossingShapeThenQuery)
{
    BoundingBox boundary(0, 0, 100, 100);

    Polygon polygon1({
        Point(10, 10),
        Point(20, 10),
        Point(20, 20),
        Point(10, 20)
        });
    Shape shape1(1, Layer::Metal1, std::move(polygon1));

    Polygon polygon2({
        Point(40, 60),
        Point(60, 60),
        Point(60, 80),
        Point(40, 80)
        });
    Shape shape2(2, Layer::Metal1, std::move(polygon2));

    Polygon polygon3({
        Point(30, 60),
        Point(40, 60),
        Point(40, 80),
        Point(30, 80)
        });
    Shape shape3(3, Layer::Metal1, std::move(polygon3));

    Polygon polygon4({
        Point(70, 60),
        Point(85, 60),
        Point(85, 80),
        Point(70, 80)
        });
    Shape shape4(4, Layer::Metal1, std::move(polygon4));

    QuadTree quadtree(boundary, 1, 5);

    BoundingBox region1(41, 65, 45, 75);
    BoundingBox region2(55, 65, 59, 75);

    const std::vector<Shape> shapes{ shape1, shape2, shape3, shape4 };
    for (const Shape& shape : shapes) {
        quadtree.insert(shape);
    }

    std::vector<const Shape*> results1;
    results1 = quadtree.query(region1);
    std::vector<const Shape*> results2;
    results2 = quadtree.query(region2);

    ASSERT_EQ(results1.size(), 1);
    EXPECT_EQ(results1[0]->getId(), 2);
    ASSERT_EQ(results2.size(), 1);
    EXPECT_EQ(results2[0]->getId(), 2);
}

TEST(QuadTreeTest, InsertShapeOutsideBoundary)
{
    BoundingBox boundary(0, 0, 100, 100);

    Polygon polygon({
        Point(150, 150),
        Point(160, 150),
        Point(160, 160),
        Point(150, 160)
        });
    Shape shape(1, Layer::Metal1, std::move(polygon));

    QuadTree quadtree(boundary, 2, 5);

    EXPECT_THROW(quadtree.insert(shape), std::invalid_argument);
}

TEST(QuadTreeTest, QueryEmptyTreeReturnsNoShapes)
{
    BoundingBox boundary(0, 0, 100, 100);
    QuadTree quadtree(boundary, 2, 5);

    BoundingBox region(0, 0, 50, 50);

    const auto results = quadtree.query(region);

    EXPECT_TRUE(results.empty());
}

TEST(QuadTreeTest, SupportsRecursiveSubdivisionBeyondRoot)
{
    BoundingBox boundary(0, 0, 100, 100);

    Polygon polygon1({
        Point(10, 10),
        Point(20, 10),
        Point(20, 20),
        Point(10, 20)
        });
    Shape shape1(1, Layer::Metal1, std::move(polygon1));

    Polygon polygon2({
        Point(20, 20),
        Point(30, 20),
        Point(30, 30),
        Point(20, 30)
        });
    Shape shape2(2, Layer::Metal1, std::move(polygon2));

    Polygon polygon3({
        Point(30, 30),
        Point(40, 30),
        Point(40, 40),
        Point(30, 40)
        });
    Shape shape3(3, Layer::Metal1, std::move(polygon3));

    QuadTree quadtree(boundary, 1, 5);

    const std::vector<Shape> shapes{ shape1, shape2, shape3 };
    for (const Shape& shape : shapes) {
        quadtree.insert(shape);
    }

    BoundingBox region(0, 0, 50, 50);

    const auto results = quadtree.query(region);

    ASSERT_EQ(results.size(), 3);
}

TEST(QuadTreeTest, InsertOneShapeTouchingBoundary)
{
    BoundingBox boundary(0, 0, 100, 100);

    Polygon polygon({
        Point(0, 0),
        Point(100, 0),
        Point(100, 100),
        Point(0, 100)
        });
    Shape shape(1, Layer::Metal1, std::move(polygon));

    QuadTree quadtree(boundary, 2, 5);

    BoundingBox region(0, 0, 100, 100);

    quadtree.insert(shape);

    std::vector<const Shape*> results;
    results = quadtree.query(region);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->getId(), 1);
}