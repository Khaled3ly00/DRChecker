#include <gtest/gtest.h>

#include "drcheck/geometry/BoundingBox.h"

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