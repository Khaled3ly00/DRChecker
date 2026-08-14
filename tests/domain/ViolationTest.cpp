#include <gtest/gtest.h>
#include "drcheck/domain/Violation.h"

using drcheck::domain::Violation;
using drcheck::domain::ViolationType;

TEST(ViolationTest, StoresViolationInformation)
{
    Violation violation (ViolationType::MinSpacing,{ 17, 42 },"Minimum spacing violation", 2.0, 3.0);

    EXPECT_EQ(violation.getType(), ViolationType::MinSpacing);
    ASSERT_EQ(violation.getShapeIds().size(), 2);
    EXPECT_EQ(violation.getShapeIds()[0], 17);
    EXPECT_EQ(violation.getShapeIds()[1], 42);
    EXPECT_EQ(violation.getMessage(), "Minimum spacing violation");
    EXPECT_DOUBLE_EQ(violation.getActualValue(), 2.0);
    EXPECT_DOUBLE_EQ(violation.getRequiredValue(), 3.0);
}