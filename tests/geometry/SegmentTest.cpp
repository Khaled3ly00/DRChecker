#include <gtest/gtest.h>

#include "drcheck/geometry/Segment.h"
#include "drcheck/geometry/Constants.h"

#include <cmath>
#include <stdexcept>

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

TEST(SegmentTest, DoesntDetectCollinearOverlapWhenBoundaryContactDisabled)
{
    Segment first(
        Point(0.0, 0.0),
        Point(6.0, 0.0)
    );

    Segment second(
        Point(3.0, 0.0),
        Point(10.0, 0.0)
    );

    EXPECT_FALSE(first.intersects(second, false));
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

    const auto result = segment.distanceTo(point);

    EXPECT_NEAR(result.distance, 3.0,EPSILON);
    EXPECT_NEAR(result.firstPoint.getX(), 4.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getY(), 0.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), 4.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getY(), 3.0, EPSILON);
}

TEST(SegmentTest, CalculatesPointDistanceBeforeStart)
{
    Segment segment(
        Point(0.0, 0.0),
        Point(10.0, 0.0)
    );

    Point point(-3.0, 4.0);

    const auto result = segment.distanceTo(point);

    EXPECT_NEAR(result.distance, 5.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getX(), 0.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getY(), 0.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), -3.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getY(), 4.0, EPSILON);
}

TEST(SegmentTest, CalculatesPointDistanceAfterEnd)
{
    Segment segment(
        Point(0.0, 0.0),
        Point(10.0, 0.0)
    );

    Point point(13.0, 4.0);

    const auto result = segment.distanceTo(point);

    EXPECT_NEAR(result.distance, 5.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getX(), 10.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getY(), 0.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), 13.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getY(), 4.0, EPSILON);
}

TEST(SegmentTest, PointOnSegmentHasZeroDistance)
{
    Segment segment(
        Point(0.0, 0.0),
        Point(10.0, 0.0)
    );

    Point point(5.0, 0.0);

    const auto result = segment.distanceTo(point);

    EXPECT_NEAR(result.distance, 0.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getX(), 5.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getY(), 0.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), 5.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getY(), 0.0, EPSILON);
}

TEST(SegmentTest, CalculatesPointDistanceFromInclinedSegment)
{
    Segment segment(
        Point(0.0, 0.0),
        Point(10.0, 10.0)
    );

    Point point(10.0, 0.0);

    const auto result = segment.distanceTo(point);

    EXPECT_NEAR(result.distance, std::sqrt(50.0), EPSILON);
    EXPECT_NEAR(result.firstPoint.getX(), 5.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getY(), 5.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), 10.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getY(), 0.0, EPSILON);
}

TEST(SegmentTest, DegeneratedSegmentsThrowsException) {

	EXPECT_THROW(Segment segment(
                    Point(2.0, 2.0),
                    Point(2.0, 2.0));, 
                std::invalid_argument);
}

TEST(SegmentTest, EndpointsWithinToleranceThrowException)
{
	EXPECT_THROW(
		Segment(
			Point(2.0, 2.0),
			Point(2.0 + EPSILON * 0.5, 2.0)
		),
		std::invalid_argument
	);
}

TEST(SegmentTest, PointDistanceToVerticalSegment) {
	Segment segment(
		Point(3.0, 1.0),
		Point(3.0, 5.0) // Vertical segment
	);
	Point point(6.0, 3.0);
	
    const auto result = segment.distanceTo(point);

    EXPECT_NEAR(result.distance, 3.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getX(), 3.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getY(), 3.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), 6.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getY(), 3.0, EPSILON);
}

TEST(SegmentTest, CollinearPointDistanceToSegment) {
    Segment segment(
        Point(0.0, 0.0),
        Point(10.0, 0.0)
    );

    Point point(13.0, 0.0);

    const auto result = segment.distanceTo(point);

    EXPECT_NEAR(result.distance, 3.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getX(), 10.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getY(), 0.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), 13.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getY(), 0.0, EPSILON);
}


TEST(SegmentTest, CalculatesDistanceBetweenParallelSegments)
{
    Segment first(
        Point(0.0, 0.0),
        Point(10.0, 0.0)
    );

    Segment second(
        Point(0.0, 3.0),
        Point(10.0, 3.0)
    );

    const auto result = first.distanceTo(second);

    EXPECT_NEAR(result.distance, 3.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getX(), 0.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getY(), 0.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), 0.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getY(), 3.0, EPSILON);
}


TEST(SegmentTest, CalculatesDistanceBetweenNonIntersectingSegments)
{
	Segment first(
		Point(0.0, 0.0),
		Point(5.0, 0.0)
	);
	Segment second(
		Point(8.0, 4.0),
		Point(10.0, 4.0)
	);

    const auto result = first.distanceTo(second);

    EXPECT_NEAR(result.distance, 5.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getX(), 5.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getY(), 0.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), 8.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getY(), 4.0, EPSILON);
}

TEST(SegmentTest, IntersectingSegmentsHaveZeroDistance)
{
    Segment first(
        Point(0.0, 0.0),
        Point(10.0, 10.0)
    );

    Segment second(
        Point(0.0, 10.0),
        Point(10.0, 0.0)
    );

    const auto result = first.distanceTo(second);

    EXPECT_NEAR(result.distance, 0.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getX(), 5.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getY(), 5.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), 5.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getY(), 5.0, EPSILON);
}

TEST(SegmentTest, NearTouchingSegmentsIntersectSymmetrically)
{
	Segment first(
		Point(0.0, 0.0),
		Point(1.0, 0.0)
	);

	Segment second(
		Point(1.0 + EPSILON * 0.5, 0.0),
		Point(2.0, 0.0)
	);

	EXPECT_TRUE(first.intersects(second));
	EXPECT_TRUE(second.intersects(first));
}

TEST(SegmentTest, SeparatedSegmentIntersectionIsSymmetric)
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
	EXPECT_FALSE(second.intersects(first));
}

TEST(SegmentTest, SegmentDistanceIsSymmetric)
{
    Segment first(
        Point(0.0, 0.0),
        Point(5.0, 0.0)
    );

    Segment second(
        Point(8.0, 4.0),
        Point(10.0, 4.0)
    );

    EXPECT_NEAR(first.distanceTo(second).distance, second.distanceTo(first).distance, EPSILON);
}

TEST(SegmentTest, DistanceResultPreservesPointOrderingWhenSegmentsAreReversed)
{
    Segment first(
        Point(0, 0),
        Point(10, 0)
    );

    Segment second(
        Point(4, 3),
        Point(4, 6)
    );

    const auto firstToSecond = first.distanceTo(second);
    const auto secondToFirst = second.distanceTo(first);

    EXPECT_DOUBLE_EQ(firstToSecond.distance, secondToFirst.distance);

    EXPECT_NEAR(firstToSecond.firstPoint.getX(), secondToFirst.secondPoint.getX(), EPSILON);
    EXPECT_NEAR(firstToSecond.firstPoint.getY(), secondToFirst.secondPoint.getY(), EPSILON);
    EXPECT_NEAR(firstToSecond.secondPoint.getX(), secondToFirst.firstPoint.getX(), EPSILON);
    EXPECT_NEAR(firstToSecond.secondPoint.getY(), secondToFirst.firstPoint.getY(), EPSILON);
}

TEST(SegmentTest, DistanceToTouchingSegmentsReturnsTouchPoint)
{
    Segment first(
        Point(0, 0),
        Point(5, 0)
    );

    Segment second(
        Point(5, 0),
        Point(5, 5)
    );

    const auto result = first.distanceTo(second);

    EXPECT_DOUBLE_EQ(result.distance, 0.0);
    EXPECT_NEAR(result.firstPoint.getX(), 5.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getY(), 0.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), 5.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getY(), 0.0, EPSILON);
}