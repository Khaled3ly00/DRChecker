#include <gtest/gtest.h>

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