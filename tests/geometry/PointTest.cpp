#include <gtest/gtest.h>

#include "drcheck/geometry/Constants.h"
#include "drcheck/geometry/Point.h"

using namespace drcheck::geometry;

TEST(PointTest, StoresCoordinatesCorrectly)
{
    Point point(3.0, 7.0);

    EXPECT_DOUBLE_EQ(point.getX(), 3.0);
    EXPECT_DOUBLE_EQ(point.getY(), 7.0);
}

TEST(PointTest, CreatesVectorsCorrectly)
{
    Point a(3.0, 7.0);
	Point b(4.0, 2.0);
    
	Vector vector = Point::vectorBetween(a, b);
	EXPECT_EQ(vector.getX(), 1.0);
	EXPECT_EQ(vector.getY(), -5.0);
}

TEST(PointTest, DetectsCounterClockwiseOrientation)
{
    Point a(0.0, 0.0);
    Point b(5.0, 0.0);
    Point c(5.0, 5.0);

    Orientation result =
        Point::getOrientation(a, b, c);

    EXPECT_EQ(
        result,
        Orientation::CounterClockwise
    );
}

TEST(PointTest, DetectsClockwiseOrientation)
{
    Point a(0.0, 0.0);
    Point b(5.0, 0.0);
    Point c(5.0, -5.0);

    Orientation result =
        Point::getOrientation(a, b, c);

    EXPECT_EQ(
        result,
        Orientation::Clockwise
    );
}

TEST(PointTest, DetectsCollinearPoints)
{
    Point a(0.0, 0.0);
    Point b(5.0, 5.0);
    Point c(10.0, 10.0);

    Orientation result =
        Point::getOrientation(a, b, c);

    EXPECT_EQ(
        result,
        Orientation::Collinear
    );
}

TEST(PointTest, DetectsPointsWithinGeometryTolerance)
{
    const Point point(2.0, 3.0);
    const Point nearby(
        2.0 + EPSILON * 0.5,
        3.0 - EPSILON * 0.5
    );

    EXPECT_TRUE(point.isNear(nearby));
    EXPECT_TRUE(nearby.isNear(point));
}

TEST(PointTest, RejectsPointsOutsideGeometryTolerance)
{
    const Point point(2.0, 3.0);
    const Point separated(
        2.0 + EPSILON * 2.0,
        3.0
    );

    EXPECT_FALSE(point.isNear(separated));
    EXPECT_FALSE(separated.isNear(point));
}

TEST(PointTest, ScalesOrientationToleranceForSmallGeometry)
{
    const Point a(0.0, 0.0);
    const Point b(1e-5, 0.0);
    const Point c(0.0, 1e-5);

    EXPECT_EQ(
        Point::getOrientation(a, b, c),
        Orientation::CounterClockwise
    );
}
