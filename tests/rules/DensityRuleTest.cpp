#include <gtest/gtest.h>
#include <stdexcept>

#include "drcheck/rules/DensityRule.h"
#include "drcheck/geometry/Constants.h"
#include "drcheck/domain/LayerRegistry.h"

using drcheck::geometry::EPSILON;
using drcheck::rules::Rule;
using drcheck::rules::DensityRule;
using drcheck::rules::DensityLimit;
using drcheck::geometry::BoundingBox;
using drcheck::geometry::Polygon;
using drcheck::geometry::Point;
using drcheck::spatial::LayerSpatialIndex;
using namespace drcheck::domain;

TEST(DensityRuleTest, StoresDensityConfiguration)
{
    LayerRegistry registry;
    const Layer* M2 = registry.declare("M2");

    DensityRule ruleMax(M2, DensityLimit::Maximum, 0.70, 10.0, 5.0);
    DensityRule ruleMin(M2, DensityLimit::Minimum, 0.30, 10.0, 5.0);

    EXPECT_EQ(ruleMax.getLimit(), DensityLimit::Maximum);
    EXPECT_NEAR(ruleMax.getRequiredDensity(), 0.70, EPSILON);
    EXPECT_EQ(ruleMin.getLimit(), DensityLimit::Minimum);
    EXPECT_NEAR(ruleMin.getRequiredDensity(), 0.30, EPSILON);
}

TEST(DensityRuleTest, StoresExplicitAnalysisWindow)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    BoundingBox region(10, 20, 40, 60);

    DensityRule rule(M1, DensityLimit::Minimum, 0.40, 10.0, 5.0, region);

    ASSERT_TRUE(rule.getAnalysisWindow().has_value());
    const auto& stored = rule.getAnalysisWindow().value();
    EXPECT_NEAR(stored.getMinX(), 10.0, EPSILON);
    EXPECT_NEAR(stored.getMinY(), 20.0, EPSILON);
    EXPECT_NEAR(stored.getMaxX(), 40.0, EPSILON);
    EXPECT_NEAR(stored.getMaxY(), 60.0, EPSILON);
}

TEST(DensityRuleTest, RejectsInvalidRequiredDensity)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    EXPECT_THROW(DensityRule(M1, DensityLimit::Minimum, -0.1, 10.0, 5.0), std::invalid_argument);
    EXPECT_THROW(DensityRule(M1, DensityLimit::Maximum, 1.1, 10.0, 5.0), std::invalid_argument);
}

TEST(DensityRuleTest, AcceptsValidRequiredDensity)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    EXPECT_NO_THROW(DensityRule(M1, DensityLimit::Minimum, 0.0, 10.0, 5.0));
    EXPECT_NO_THROW(DensityRule(M1, DensityLimit::Maximum, 1.0, 10.0, 5.0));
}

TEST(DensityRuleTest, RejectsInvalidWindowSizeORStep)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    EXPECT_THROW(DensityRule(M1, DensityLimit::Minimum, 0.5, -1.0, 5.0), std::invalid_argument);
    EXPECT_THROW(DensityRule(M1, DensityLimit::Maximum, 0.8, 10.0, -5.0), std::invalid_argument);
}

TEST(DensityRuleTest, AcceptsExactMinimumDensity)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    Polygon polygon({
        Point(0, 0),
        Point(3, 0),
        Point(3, 10),
        Point(0, 10)
        });

    std::vector<Shape> shapes;
    shapes.emplace_back(1, M1, std::move(polygon));

    LayerSpatialIndex spatialIndex(shapes);

    DensityRule rule(M1, DensityLimit::Minimum, 0.30, 10.0, 10.0, BoundingBox(0, 0, 10, 10));

    const auto violations = rule.check(shapes, spatialIndex);
    EXPECT_TRUE(violations.empty());
}

TEST(DensityRuleTest, RejectsBelowMinimumDensity)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    Polygon polygon({
        Point(0, 0),
        Point(2, 0),
        Point(2, 10),
        Point(0, 10)
        });

    std::vector<Shape> shapes;
    shapes.emplace_back(1, M1, std::move(polygon));
    LayerSpatialIndex spatialIndex(shapes);

    DensityRule rule(M1, DensityLimit::Minimum, 0.30, 10.0, 10.0, BoundingBox(0, 0, 10, 10));

    const auto violations = rule.check(shapes, spatialIndex);

    ASSERT_EQ(violations.size(), 1);
    EXPECT_EQ(violations[0].getType(), ViolationType::MinDensity);
    EXPECT_NEAR(violations[0].getActualValue(), 0.20, EPSILON);
    EXPECT_NEAR(violations[0].getRequiredValue(), 0.30, EPSILON);
}

TEST(DensityRuleTest, AcceptsExactMaximumDensity)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    Polygon polygon({
        Point(0, 0),
        Point(7, 0),
        Point(7, 10),
        Point(0, 10)
        });

    std::vector<Shape> shapes;
    shapes.emplace_back(1, M1, std::move(polygon));
    LayerSpatialIndex spatialIndex(shapes);

    DensityRule rule(M1, DensityLimit::Maximum, 0.70, 10.0, 10.0, BoundingBox(0, 0, 10, 10));
    const auto violations = rule.check(shapes, spatialIndex);
    EXPECT_TRUE(violations.empty());
}

TEST(DensityRuleTest, RejectsAboveMaximumDensity)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    Polygon polygon({
        Point(0, 0),
        Point(8, 0),
        Point(8, 10),
        Point(0, 10)
        });

    std::vector<Shape> shapes;
    shapes.emplace_back(1, M1, std::move(polygon));

    LayerSpatialIndex spatialIndex(shapes);

    DensityRule rule(M1, DensityLimit::Maximum, 0.70, 10.0, 10.0, BoundingBox(0, 0, 10, 10));
    const auto violations = rule.check(shapes, spatialIndex);

    ASSERT_EQ(violations.size(), 1);
    EXPECT_EQ(violations[0].getType(), ViolationType::MaxDensity);
    EXPECT_NEAR(violations[0].getActualValue(), 0.80, EPSILON);
}

TEST(DensityRuleTest, IgnoresShapesOnOtherLayers)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* M2 = registry.declare("M2");

    Polygon metal1Polygon({
        Point(0, 0),
        Point(2, 0),
        Point(2, 10),
        Point(0, 10)
        });

    Polygon metal2Polygon({
        Point(2, 0),
        Point(10, 0),
        Point(10, 10),
        Point(2, 10)
        });

    std::vector<Shape> shapes;

    shapes.emplace_back(1, M1, std::move(metal1Polygon));
    shapes.emplace_back(2, M2, std::move(metal2Polygon));
    LayerSpatialIndex spatialIndex(shapes);
    DensityRule rule(M1, DensityLimit::Minimum, 0.50, 10.0, 10.0, BoundingBox(0, 0, 10, 10));
    const auto violations = rule.check(shapes, spatialIndex);

    ASSERT_EQ(violations.size(), 1);
    EXPECT_NEAR(violations[0].getActualValue(), 0.20, EPSILON);
}

TEST(DensityRuleTest, DefaultAnalysisWindowUsesAllLayoutShapes)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* M2 = registry.declare("M2");

    Polygon metal1Polygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });

    Polygon metal2Polygon({
        Point(20, 0),
        Point(30, 0),
        Point(30, 10),
        Point(20, 10)
        });

    std::vector<Shape> shapes;

    shapes.emplace_back(1, M1, std::move(metal1Polygon));
    shapes.emplace_back(2, M2, std::move(metal2Polygon));
    LayerSpatialIndex spatialIndex(shapes);
    DensityRule rule(M1, DensityLimit::Minimum, 0.50, 30.0, 30.0);
    const auto violations = rule.check(shapes, spatialIndex);

    ASSERT_EQ(violations.size(), 1);
    EXPECT_NEAR(violations[0].getActualValue(), 1.0 / 3.0, EPSILON);
}

TEST(DensityRuleTest, UsesExplicitAnalysisWindow)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* M2 = registry.declare("M2");

    Polygon metal1Polygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });

    Polygon metal2Polygon({
        Point(20, 0),
        Point(30, 0),
        Point(30, 10),
        Point(20, 10)
        });

    std::vector<Shape> shapes;

    shapes.emplace_back(1, M1, std::move(metal1Polygon));
    shapes.emplace_back(2, M2, std::move(metal2Polygon));
    LayerSpatialIndex spatialIndex(shapes);
    DensityRule rule(M1, DensityLimit::Minimum, 0.80, 10.0, 10.0, BoundingBox(0, 0, 10, 10));

    const auto violations = rule.check(shapes, spatialIndex);
    EXPECT_TRUE(violations.empty());
}

TEST(DensityRuleTest, PartialEdgeSampleWindowUsesActualWindowArea)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    // Ensures that a partial sample window created for remaining area 
    // and it's density is measured according to partial sample window area
    Polygon polygon({
        Point(0, 0),
        Point(20, 0),
        Point(20, 20),
        Point(0, 20)
        });

    std::vector<Shape> shapes;
    shapes.emplace_back(1, M1, std::move(polygon));
    LayerSpatialIndex spatialIndex(shapes);
    DensityRule rule(M1, DensityLimit::Minimum, 1.0, 10.0, 10.0, BoundingBox(0, 0, 23, 10));
    const auto violations = rule.check(shapes, spatialIndex);
    // First and Second windows passes, Third (partial window) violates
    ASSERT_EQ(violations.size(), 1);
    ASSERT_TRUE(violations[0].getMarker().has_value());
    const auto& marker = violations[0].getMarker().value();
    ASSERT_TRUE(marker.region.has_value());
    EXPECT_NEAR(marker.region->getMinX(), 20.0, EPSILON);
    EXPECT_NEAR(marker.region->getMaxX(), 23.0, EPSILON);
}

TEST(DensityRuleTest, SupportsOverlappingSamplingWindows)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    Polygon polygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 5),
        Point(0, 5)
        });

    std::vector<Shape> shapes;
    shapes.emplace_back(1, M1, std::move(polygon));
    LayerSpatialIndex spatialIndex(shapes);
    DensityRule rule(M1, DensityLimit::Minimum, 1.0, 10.0, 5.0, BoundingBox(0, 0, 15, 5));
    const auto violations = rule.check(shapes, spatialIndex);
    //Window 1: x 0–10      density = 100 %         PASS
    //Window 2: x 5–15      density = 50 %          FAIL
    //Window 3: x 10–15     density = 0 %           FAIL
    ASSERT_EQ(violations.size(), 2);
    ASSERT_TRUE(violations[0].getMarker().has_value());
    const auto& marker = violations[0].getMarker().value();
    EXPECT_EQ(marker.region->getMinX(), 5.0);
    EXPECT_EQ(marker.region->getMinY(), 0.0);
    EXPECT_EQ(marker.region->getMaxX(), 15.0);
    EXPECT_EQ(marker.region->getMaxY(), 5.0);
}

TEST(DensityRuleTest, ExplicitAnalysisWindowSupportsEmptyLayout)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    const std::vector<Shape> shapes;
    const LayerSpatialIndex spatialIndex(shapes);

    DensityRule rule(M1, DensityLimit::Minimum, 0.30, 10.0, 10.0, BoundingBox(0, 0, 20, 10));

    const auto violations = rule.check(shapes, spatialIndex);

    ASSERT_EQ(violations.size(), 2);

    EXPECT_NEAR(violations[0].getActualValue(), 0.0, EPSILON);
    EXPECT_NEAR(violations[1].getActualValue(), 0.0, EPSILON);
}

TEST(DensityRuleTest, RejectsEmptyLayoutWithoutExplicitAnalysisWindow)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    const std::vector<Shape> shapes;
    const LayerSpatialIndex spatialIndex(shapes);

    DensityRule rule(M1, DensityLimit::Minimum, 0.30, 10.0, 10.0);

    EXPECT_THROW(rule.check(shapes, spatialIndex), std::invalid_argument);
}
