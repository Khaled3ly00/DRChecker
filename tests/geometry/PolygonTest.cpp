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

TEST(PolygonTest, ContainsPointInside)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 5.0),
        Point(0.0, 5.0)
        });

    Point point(5.0, 2.0);

    EXPECT_TRUE(polygon.contains(point));
}

TEST(PolygonTest, RejectsPointOutside)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 5.0),
        Point(0.0, 5.0)
        });

    Point point(15.0, 2.0);

    EXPECT_FALSE(polygon.contains(point));
}

TEST(PolygonTest, ContainsPointAtVertex)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 5.0),
        Point(0.0, 5.0)
        });

    Point point(10.0, 5.0);

    EXPECT_TRUE(polygon.contains(point));
}

TEST(PolygonTest, ContainsPointOnEdge)
{
	Polygon polygon({
		Point(0.0, 0.0),
		Point(10.0, 0.0),
		Point(10.0, 5.0),
		Point(0.0, 5.0)
		});
	Point point(5.0, 0.0);
	EXPECT_TRUE(polygon.contains(point));
}

TEST(PolygonTest, ContainsPointInConcavePolygon) {
	Polygon polygon({
		Point(0.0, 0.0),
		Point(5.0, 5.0),
		Point(10.0, 0.0),
		Point(10.0, 10.0),
		Point(0.0, 10.0)
		});
	Point point(4.0, 4.0);
	EXPECT_TRUE(polygon.contains(point));
}

TEST(PolygonTest, DoesNotContainPointInConcavePolygon) {
    Polygon polygon({
        Point(0.0, 0.0),
        Point(5.0, 5.0),
        Point(10.0, 0.0),
        Point(10.0, 10.0),
        Point(0.0, 10.0)
        });
    Point point(5.0, 2.0);
    EXPECT_FALSE(polygon.contains(point));
}

TEST(PolygonTest, DetectsOverlappingPolygons)
{
    Polygon first({
        Point(0.0, 0.0),
        Point(6.0, 0.0),
        Point(6.0, 4.0),
        Point(0.0, 4.0)
        });

    Polygon second({
        Point(4.0, 2.0),
        Point(10.0, 2.0),
        Point(10.0, 6.0),
        Point(4.0, 6.0)
        });

    EXPECT_TRUE(first.intersects(second));
}

TEST(PolygonTest, SeparatedPolygonsDoNotIntersect)
{
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

    EXPECT_FALSE(first.intersects(second));
}

TEST(PolygonTest, DetectsContainedPolygon)
{
    Polygon outer({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 10.0),
        Point(0.0, 10.0)
        });

    Polygon inner({
        Point(3.0, 3.0),
        Point(7.0, 3.0),
        Point(7.0, 7.0),
        Point(3.0, 7.0)
        });

    EXPECT_TRUE(outer.intersects(inner));
}

TEST(PolygonTest, DetectsContainedPolygonReverseCall)
{
    Polygon outer({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 10.0),
        Point(0.0, 10.0)
        });

    Polygon inner({
        Point(3.0, 3.0),
        Point(7.0, 3.0),
        Point(7.0, 7.0),
        Point(3.0, 7.0)
        });

    EXPECT_TRUE(inner.intersects(outer));
}

TEST(PolygonTest, IntersectionIsSymmetric)
{
    Polygon first({
        Point(0.0, 0.0),
        Point(6.0, 0.0),
        Point(6.0, 6.0),
        Point(0.0, 6.0)
        });

    Polygon second({
        Point(4.0, 4.0),
        Point(8.0, 4.0),
        Point(8.0, 8.0),
        Point(4.0, 8.0)
        });

    EXPECT_EQ(
        first.intersects(second),
        second.intersects(first)
    );
}

TEST(PolygonTest, TouchingEdgesCountAsIntersection)
{
    Polygon first({
        Point(0.0, 0.0),
        Point(5.0, 0.0),
        Point(5.0, 5.0),
        Point(0.0, 5.0)
        });

    Polygon second({
        Point(5.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 5.0),
        Point(5.0, 5.0)
        });

    EXPECT_TRUE(first.intersects(second));
}

TEST(PolygonTest, TouchingVertexCountsAsIntersection)
{
    Polygon first({
        Point(0.0, 0.0),
        Point(5.0, 0.0),
        Point(5.0, 5.0),
        Point(0.0, 5.0)
        });

    Polygon second({
        Point(5.0, 5.0),
        Point(9.0, 5.0),
        Point(9.0, 9.0),
        Point(5.0, 9.0)
        });

    EXPECT_TRUE(first.intersects(second));
}