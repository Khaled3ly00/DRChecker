#include <gtest/gtest.h>

#include "drcheck/geometry/Segment.h"
#include "drcheck/geometry/Constants.h"

using namespace drcheck::geometry;

TEST(SegmentTest, DetectsPointOnSegment) {
    Segment S(
        Point(0.0, 0.0),
        Point(6.0, 6.0)
    );
	Point testPoint(3.0, 3.0);
	EXPECT_TRUE(S.contains(testPoint));
}

TEST(SegmentTest, DetectsNonCollinearPointNotOnSegment) {
	Segment S(
		Point(0.0, 0.0),
		Point(6.0, 6.0)
	);
	Point testPoint(3.0, 4.0);
	EXPECT_FALSE(S.contains(testPoint));
}

TEST(SegmentTest, DetectsCollinearPointOutsideSegment) {
	Segment S(
		Point(0.0, 0.0),
		Point(6.0, 6.0)
	);
	Point testPoint(7.0, 7.0);
	EXPECT_FALSE(S.contains(testPoint));
}

TEST(SegmentTest, DetectsProperIntersection)
{
    Segment first(
        Point(0.0, 0.0),
        Point(6.0, 6.0)
    );

    Segment second(
        Point(0.0, 6.0),
        Point(6.0, 0.0)
    );

    EXPECT_TRUE(first.intersects(second));
}

TEST(SegmentTest, ParallelSegmentsDoNotIntersect)
{
    Segment first(
        Point(0.0, 0.0),
        Point(5.0, 0.0)
    );

    Segment second(
        Point(0.0, 3.0),
        Point(5.0, 3.0)
    );

    EXPECT_FALSE(first.intersects(second));
}

TEST(SegmentTest, DetectsCollinearOverlap)
{
    Segment first(
        Point(0.0, 0.0),
        Point(6.0, 0.0)
    );

    Segment second(
        Point(3.0, 0.0),
        Point(10.0, 0.0)
    );

    EXPECT_TRUE(first.intersects(second));
}

TEST(SegmentTest, SharedEndpointCountsAsIntersection)
{
    Segment first(
        Point(0.0, 0.0),
        Point(5.0, 0.0)
    );

    Segment second(
        Point(5.0, 0.0),
        Point(8.0, 3.0)
    );

    EXPECT_TRUE(first.intersects(second));
}

TEST(SegmentTest, CollinearSeparatedSegmentsDoNotIntersect)
{
    Segment first(
        Point(0.0, 0.0),
        Point(3.0, 0.0)
    );

    Segment second(
        Point(5.0, 0.0),
        Point(8.0, 0.0)
    );

    EXPECT_FALSE(first.intersects(second));
}

TEST(SegmentTest, CalculatesPointDistanceWithProjectionInside)
{
    Segment segment(
        Point(0.0, 0.0),
        Point(10.0, 0.0)
    );

    Point point(4.0, 3.0);

    EXPECT_NEAR(segment.distanceTo(point), 3.0, EPSILON);
}

TEST(SegmentTest, CalculatesPointDistanceBeforeStart)
{
    Segment segment(
        Point(0.0, 0.0),
        Point(10.0, 0.0)
    );

    Point point(-3.0, 4.0);

    EXPECT_NEAR(segment.distanceTo(point), 5.0, EPSILON);
}

TEST(SegmentTest, CalculatesPointDistanceAfterEnd)
{
    Segment segment(
        Point(0.0, 0.0),
        Point(10.0, 0.0)
    );

    Point point(13.0, 4.0);

    EXPECT_NEAR(segment.distanceTo(point), 5.0, EPSILON);
}

TEST(SegmentTest, PointOnSegmentHasZeroDistance)
{
    Segment segment(
        Point(0.0, 0.0),
        Point(10.0, 0.0)
    );

    Point point(5.0, 0.0);

    EXPECT_NEAR(segment.distanceTo(point), 0.0, EPSILON);
}

TEST(SegmentTest, PointDistanceToDegeneratedSegment) {
	Segment segment(
		Point(2.0, 2.0),
		Point(2.0, 2.0) // Degenerated segment (point)
	);
	Point point(5.0, 6.0);
	EXPECT_NEAR(segment.distanceTo(point), 5.0, EPSILON);
}

TEST(SegmentTest, PointDistanceToVerticalSegment) {
	Segment segment(
		Point(3.0, 1.0),
		Point(3.0, 5.0) // Vertical segment
	);
	Point point(6.0, 3.0);
	EXPECT_NEAR(segment.distanceTo(point), 3.0, EPSILON);
}

TEST(SegmentTest, PointDistanceToInclinedSegment) {
	Segment segment(
		Point(1.0, 1.0),
		Point(4.0, 4.0) // Inclined segment
	);
	Point point(2.0, 3.0);
	EXPECT_NEAR(segment.distanceTo(point), std::sqrt(2.0)/2, EPSILON);
}

TEST(SegmentTest, CollinearPointDistanceToSegment) {
    Segment segment(
        Point(0.0, 0.0),
        Point(10.0, 0.0)
    );

    Point point(13.0, 0.0);

    EXPECT_NEAR(segment.distanceTo(point), 3.0, EPSILON);
}