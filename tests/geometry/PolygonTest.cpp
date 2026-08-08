#include <gtest/gtest.h>

#include "drcheck/geometry/Polygon.h"

using namespace drcheck::geometry;

TEST(PolygonTest, InitializesWithVerticesAndGetsCorrectVertexCount)
{
    std::vector<Point> vertices = {
        Point(0.0, 0.0),
        Point(3.0, 0.0),
        Point(3.0, 4.0)
    };

    Polygon polygon(vertices);

    EXPECT_EQ(polygon.getVertexCount(), 3);
}

TEST(PolygonTest, RejectsFewerThanThreeVertices)
{
    EXPECT_THROW(
        Polygon({
            Point(0.0, 0.0),
            Point(5.0, 0.0)
            }),
        std::invalid_argument
    );
}

TEST(PolygonTest, GetsCorrectEdges)
{
    std::vector<Point> vertices = {
        Point(0.0, 0.0),
        Point(3.0, 0.0),
        Point(3.0, 4.0)
    };

    Polygon polygon(vertices);

    std::vector<Segment> edges = polygon.getEdges();
    EXPECT_EQ(edges.size(), 3);
}

TEST(PolygonTest, CalculatesRectangleArea)
{
    std::vector<Point> vertices = {
        Point(0.0, 0.0),
        Point(8.0, 0.0),
        Point(8.0, 5.0),
        Point(0.0, 5.0)
    };
    Polygon polygon(vertices);

    EXPECT_DOUBLE_EQ(polygon.area(), 40.0);
}

TEST(PolygonTest, DetectsCounterClockwiseOrientation)
{
    std::vector<Point> vertices = {
        Point(0.0, 0.0),
        Point(8.0, 0.0),
        Point(8.0, 5.0),
        Point(0.0, 5.0)
    };
    Polygon polygon(vertices);

    EXPECT_EQ(polygon.getOrientation(), Orientation::CounterClockwise);
}

TEST(PolygonTest, DetectsClockwiseOrientation)
{
    std::vector<Point> vertices = {
        Point(0.0, 0.0),
        Point(0.0, 5.0),
        Point(8.0, 5.0),
        Point(8.0, 0.0)
    };
    Polygon polygon(vertices);

    EXPECT_EQ(polygon.getOrientation(), Orientation::Clockwise);
}

TEST(PolygonTest, CalculatesBoundingBox)
{   
    std::vector<Point> vertices = {
        Point(2.0, 1.0),
        Point(8.0, 3.0),
        Point(6.0, 9.0),
        Point(1.0, 6.0)
    };
    Polygon polygon(vertices);

    BoundingBox box = polygon.getBoundingBox();

    EXPECT_DOUBLE_EQ(box.getMinX(), 1.0);
    EXPECT_DOUBLE_EQ(box.getMaxX(), 8.0);

    EXPECT_DOUBLE_EQ(box.getMinY(), 1.0);
    EXPECT_DOUBLE_EQ(box.getMaxY(), 9.0);
}