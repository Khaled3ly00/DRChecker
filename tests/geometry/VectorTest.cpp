#include <gtest/gtest.h>

#include "drcheck/geometry/Vector.h"
#include "drcheck/geometry/Constants.h"

using namespace drcheck::geometry;

TEST(VectorTest, CalculatesLength)
{
    Vector v(3.0, 4.0);

    EXPECT_DOUBLE_EQ(v.length(), 5.0);
}

TEST(VectorTest, CalculatesDotProduct)
{
    Vector a(3.0, 2.0);
    Vector b(4.0, 1.0);

    EXPECT_DOUBLE_EQ(a.dot(b), 14.0);
}

TEST(VectorTest, CalculatesCrossProduct)
{
    Vector a(1.0, 0.0);
    Vector b(0.0, 1.0);

    EXPECT_DOUBLE_EQ(a.cross(b), 1.0);
}

TEST(VectorTest, NormalizedVectorPreservesDirection)
{
    Vector vector(3.0, 4.0);

    const Vector normalized = vector.normalized();

    EXPECT_NEAR(normalized.length(), 1.0, EPSILON);
    EXPECT_NEAR(normalized.getX(), 0.6, EPSILON);
    EXPECT_NEAR(normalized.getY(), 0.8, EPSILON);
}

TEST(VectorTest, NormalizingZeroLengthVectorThrows)
{
    Vector vector(0.0, 0.0);

    EXPECT_THROW(vector.normalized(), std::logic_error);
}