#include <gtest/gtest.h>

#include "drcheck/geometry/Segment.h"

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