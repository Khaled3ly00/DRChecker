#include <gtest/gtest.h>

#include "drcheck/geometry/BoundingBox.h"
#include "drcheck/geometry/Constants.h"

#include <stdexcept>

using namespace drcheck::geometry;

TEST(BoundingBoxTest, DetectsOverlappingBoxes)
{
    BoundingBox first(
        0.0, 0.0,
        5.0, 5.0
    );

    BoundingBox second(
        3.0, 3.0,
        8.0, 8.0
    );

    EXPECT_TRUE(first.overlaps(second));
}

TEST(BoundingBoxTest, DetectsSeparatedBoxes)
{
    BoundingBox first(
        0.0, 0.0,
        5.0, 5.0
    );

    BoundingBox second(
        10.0, 10.0,
        15.0, 15.0
    );

    EXPECT_FALSE(first.overlaps(second));
}

TEST(BoundingBoxTest, TouchingEdgesCountAsOverlap)
{
    BoundingBox first(
        0.0, 0.0,
        5.0, 5.0
    );

    BoundingBox second(
        5.0, 0.0,
        10.0, 5.0
    );

    EXPECT_TRUE(first.overlaps(second));
}

TEST(BoundingBoxTest, SupportsExplicitOverlapTolerance)
{
    const BoundingBox first(
        0.0, 0.0,
        1.0, 1.0
    );

    const BoundingBox second(
        1.0 + EPSILON * 0.5, 0.0,
        2.0, 1.0
    );

    EXPECT_FALSE(first.overlaps(second));
    EXPECT_TRUE(first.overlaps(second, EPSILON));
    EXPECT_TRUE(second.overlaps(first, EPSILON));
}

TEST(BoundingBoxTest, RejectsInvertedCoordinateRanges)
{
    EXPECT_THROW(
        BoundingBox(5.0, 0.0, 1.0, 4.0),
        std::invalid_argument
    );

    EXPECT_THROW(
        BoundingBox(0.0, 5.0, 4.0, 1.0),
        std::invalid_argument
    );
}

TEST(BoundingBoxTest, RejectsNegativeOverlapTolerance)
{
    const BoundingBox first(0.0, 0.0, 1.0, 1.0);
    const BoundingBox second(2.0, 0.0, 3.0, 1.0);

    EXPECT_THROW(
        first.overlaps(second, -EPSILON),
        std::invalid_argument
    );
}

TEST(BoundingBoxTest, ContainsInnerBoundingBox)
{
    BoundingBox outer(0, 0, 100, 100);
    BoundingBox inner(20, 20, 40, 40);

    EXPECT_TRUE(outer.contains(inner));
}

TEST(BoundingBoxTest, ContainsBoundingBoxTouchingBoundary)
{
    BoundingBox outer(0, 0, 100, 100);
    BoundingBox inner(0, 20, 40, 40);

    EXPECT_TRUE(outer.contains(inner));
}

TEST(BoundingBoxTest, DoesNotContainPartiallyOutsideBoundingBox)
{
    BoundingBox outer(0, 0, 100, 100);
    BoundingBox other(90, 20, 110, 40);

    EXPECT_FALSE(outer.contains(other));
}

TEST(BoundingBoxTest, MergesBoundingBoxes)
{
    BoundingBox first(0, 0, 10, 10);
    BoundingBox second(-5, 5, 30, 20);

    const BoundingBox result = first.mergedWith(second);

    EXPECT_DOUBLE_EQ(result.getMinX(), -5);
    EXPECT_DOUBLE_EQ(result.getMinY(), 0);
    EXPECT_DOUBLE_EQ(result.getMaxX(), 30);
    EXPECT_DOUBLE_EQ(result.getMaxY(), 20);
}

TEST(BoundingBoxTest, ExpandsBoundingBox)
{
    BoundingBox box(10, 20, 30, 40);
    const BoundingBox expanded = box.expanded(5);

    EXPECT_DOUBLE_EQ(expanded.getMinX(), 5);
    EXPECT_DOUBLE_EQ(expanded.getMinY(), 15);
    EXPECT_DOUBLE_EQ(expanded.getMaxX(), 35);
    EXPECT_DOUBLE_EQ(expanded.getMaxY(), 45);
}

TEST(BoundingBoxTest, RejectsNegativeExpansion)
{
    BoundingBox box(0, 0, 10, 10);

    EXPECT_THROW(box.expanded(-1), std::invalid_argument);
}