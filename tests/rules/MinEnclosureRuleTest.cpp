#include <gtest/gtest.h>

#include "drcheck/rules/MinEnclosureRule.h"
#include "drcheck/geometry/Constants.h"
#include "drcheck/domain/LayerRegistry.h"

#include <memory>

using drcheck::geometry::EPSILON;
using drcheck::rules::Rule;
using drcheck::rules::MinEnclosureRule;
using drcheck::rules::EnclosureOption;
using drcheck::geometry::Polygon;
using drcheck::geometry::Point;
using namespace drcheck::domain;

TEST(MinEnclosureRuleTest, AcceptsExactMinEnclosure)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");

    Polygon firstPolygon({
        Point(0,0),
        Point(10,0),
        Point(10,10),
        Point(0,10)
        });
    Polygon secondPolygon({
        Point(2,2),
        Point(2,8),
        Point(8,8),
        Point(8,2)
        });

    Shape first(1, M1, std::move(firstPolygon));
    Shape second(2, VIA1, std::move(secondPolygon));
    MinEnclosureRule rule(VIA1, M1, 2.0);
    const std::vector<Shape> shapes{first, second};
    // Creates QuadTree
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}

TEST(MinEnclosureRuleTest, AcceptsMoreThanMinEnclosure)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");

    Polygon firstPolygon({
        Point(0,0),
        Point(15,0),
        Point(15,15),
        Point(0,15)
        });
    Polygon secondPolygon({
        Point(5,5),
        Point(5,12),
        Point(12,12),
        Point(12,5)
        });

    Shape first(1, M1, std::move(firstPolygon));
    Shape second(2, VIA1, std::move(secondPolygon));
    MinEnclosureRule rule(VIA1, M1, 2.0);
    const std::vector<Shape> shapes{ first, second };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}

TEST(MinEnclosureRuleTest, RejectsLessThanMinEnclosure)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");

    Polygon firstPolygon({
        Point(0,0),
        Point(10,0),
        Point(10,10),
        Point(0,10)
        });
    Polygon secondPolygon({
        Point(2,2),
        Point(2,8),
        Point(8,8),
        Point(8,2)
        });

    Shape first(1, M1, std::move(firstPolygon));
    Shape second(2, VIA1, std::move(secondPolygon));
    MinEnclosureRule rule(VIA1, M1, 3.0);
    const std::vector<Shape> shapes{ first, second };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_EQ(violations.size(), 1);
    EXPECT_NEAR(violations[0].getActualValue(), 2.0, EPSILON);
    EXPECT_NEAR(violations[0].getRequiredValue(), 3.0, EPSILON);
    EXPECT_EQ(violations[0].getShapeIds()[0], 2);
    EXPECT_EQ(violations[0].getShapeIds()[1], 1);
    // Marker tests
    ASSERT_TRUE(violations[0].getMarker().has_value());
    const auto& marker = violations[0].getMarker().value();
    // First nearst edge (point)
    ASSERT_TRUE(marker.firstPoint.has_value());
    ASSERT_TRUE(marker.secondPoint.has_value());
    EXPECT_NEAR(Point::vectorBetween(marker.firstPoint.value(), marker.secondPoint.value()).length(), 2.0, EPSILON);
}

TEST(MinEnclosureRuleTest, IntersectingAllowedOuterLayerReportsZeroEnclosure) // Intersecting polygon should return violation as enclosure distance is zero
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");

    Polygon firstPolygon({
        Point(0.0, 0.0),
        Point(6.0, 0.0),
        Point(6.0, 6.0),
        Point(0.0, 6.0)
        });

    Polygon secondPolygon({
        Point(4.0, 4.0),
        Point(8.0, 4.0),
        Point(8.0, 8.0),
        Point(4.0, 8.0)
        });

    Shape first(1, VIA1, std::move(firstPolygon));
    Shape second(2, M1, std::move(secondPolygon));
    MinEnclosureRule rule(VIA1, M1, 1.0);
    const std::vector<Shape> shapes{first, second};
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_EQ(violations.size(), 1);    
    EXPECT_NEAR(violations[0].getActualValue(), 0.0, EPSILON);
    // Marker tests
    ASSERT_TRUE(violations[0].getMarker().has_value());
}

TEST(MinEnclosureRuleTest, TouchingInternallyPolygonsMinEnclosure) // Touching Internally polygon should return violation as enclosure distance is zero
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");

    Polygon firstPolygon({
        Point(0.0, 0.0),
        Point(6.0, 0.0),
        Point(6.0, 6.0),
        Point(0.0, 6.0)
        });

    Polygon secondPolygon({
        Point(4.0, 4.0),
        Point(6.0, 4.0),
        Point(6.0, 6.0),
        Point(4.0, 6.0)
        });

    Shape first(1, M1, std::move(firstPolygon));
    Shape second(2, VIA1, std::move(secondPolygon));
    MinEnclosureRule rule(VIA1, M1, 1.0);
    const std::vector<Shape> shapes{ first, second };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_EQ(violations.size(), 1);
}

TEST(MinEnclosureRuleTest, InnerCompletelyOutsideMinEnclosure) {
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");

    Polygon firstPolygon({
        Point(0.0, 0.0),
        Point(6.0, 0.0),
        Point(6.0, 6.0),
        Point(0.0, 6.0)
        });

    Polygon secondPolygon({
        Point(8.0, 8.0),
        Point(12.0, 8.0),
        Point(12.0, 12.0),
        Point(8.0, 12.0)
        });

    Shape first(1, VIA1, std::move(firstPolygon));
    Shape second(2, M1, std::move(secondPolygon));
    MinEnclosureRule rule(VIA1, M1, 2.0);
    const std::vector<Shape> shapes{ first, second };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_EQ(violations.size(), 1);
}


TEST(MinEnclosureRuleTest, InnerCompletelyInsideConcavePolygonMinEnclosure) {
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");

    Polygon firstPolygon({
        Point(2.0, 4.0),
        Point(2.0, 6.0),
        Point(4.0, 6.0),
        Point(4.0, 4.0)
        });

    Polygon secondPolygon({
        Point(0.0, 0.0),
        Point(2.0, 0.0),
        Point(2.0, 2.0),
        Point(4.0, 2.0),
        Point(4.0, 0.0),
        Point(6.0, 0.0),
        Point(6.0, 8.0),
        Point(0.0, 8.0)
        });

    Shape first(1, VIA1, std::move(firstPolygon));
    Shape second(2, M1, std::move(secondPolygon));
    MinEnclosureRule rule(VIA1, M1, 2.0);
    const std::vector<Shape> shapes{ first, second };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}

TEST(MinEnclosureRuleTest, InnerIntersectsOuterConcavePolygonMinEnclosure) {
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");

    Polygon firstPolygon({
        Point(2.0, 1.0),
        Point(2.0, 6.0),
        Point(4.0, 6.0),
        Point(4.0, 1.0)
        });

    Polygon secondPolygon({
        Point(0.0, 0.0),
        Point(2.0, 0.0),
        Point(2.0, 2.0),
        Point(4.0, 2.0),
        Point(4.0, 0.0),
        Point(6.0, 0.0),
        Point(6.0, 8.0),
        Point(0.0, 8.0)
        });

    Shape first(1, VIA1, std::move(firstPolygon));
    Shape second(2, M1, std::move(secondPolygon));
    MinEnclosureRule rule(VIA1, M1, 3.0);
    const std::vector<Shape> shapes{ first, second };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_EQ(violations.size(), 1);
}
// If the rule checks for via12 enclosure within Metal 1
// But there's only 2 shapes: metal 2 and via12
// Then if rule is checked a violation appears as there's no via12 enclosed within M1
TEST(MinEnclosureRuleTest,NoMatchingOuterLayerMinEnclosure)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* M2 = registry.declare("M2");
    const Layer* VIA1 = registry.declare("VIA1");

    Polygon firstPolygon({
        Point(0,0),
        Point(10,0),
        Point(10,10),
        Point(0,10)
        });
    Polygon secondPolygon({
        Point(2,2),
        Point(2,8),
        Point(8,8),
        Point(8,2)
        });

    Shape first(1, M2, std::move(firstPolygon));
    Shape second(2, VIA1, std::move(secondPolygon));
    MinEnclosureRule rule(VIA1, M1, 2.0);
    const std::vector<Shape> shapes{ first, second };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_EQ(violations.size(), 1);
}
// Multiple outer candidates, one valid. Have two M1 shapes:
// one does not contain the VIA1, while another provides sufficient enclosure.
TEST(MinEnclosureRuleTest, MultipleOuterLayerPolygonsMinEnclosure)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");

    Polygon firstPolygon({
        Point(0,0),
        Point(5,0),
        Point(5,5),
        Point(0,5)
        });
    Polygon secondPolygon({
        Point(6,6),
        Point(6,10),
        Point(10,10),
        Point(10,6)
        });
    Polygon thirdPolygon({
        Point(2,2),
        Point(2,3),
        Point(3,3),
        Point(3,2)
        });

    Shape first(1, M1, std::move(firstPolygon));
    Shape second(2, M1, std::move(secondPolygon));
    Shape third(3, VIA1, std::move(thirdPolygon));
    MinEnclosureRule rule(VIA1, M1, 2.0);
    const std::vector<Shape> shapes{first, second, third};
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}
// Multiple inner shapes. For example, 
// one VIA1 correctly enclosed and another VIA1 insufficiently enclosed. 
// Expected result: exactly one violation
TEST(MinEnclosureRuleTest, MultipleInnerLayerPolygonsMinEnclosure)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");

    Polygon firstPolygon({
        Point(0,0),
        Point(10,0),
        Point(10,10),
        Point(0,10)
        });
    Polygon secondPolygon({
        Point(2,2),
        Point(2,3),
        Point(3,3),
        Point(3,2)
        });
    Polygon thirdPolygon({
        Point(9,9),
        Point(9,10),
        Point(10,10),
        Point(10,9)
        });
    Shape first(1, M1, std::move(firstPolygon));
    Shape second(2, VIA1, std::move(secondPolygon));
    Shape third(3, VIA1, std::move(thirdPolygon));
    MinEnclosureRule rule(VIA1, M1, 2.0);
    const std::vector<Shape> shapes{ first, second, third };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    ASSERT_EQ(violations.size(), 1);
    EXPECT_EQ(violations[0].getShapeIds()[0], 3);
}

TEST(MinEnclosureRuleTest, ThrowsOnNonPositiveMinimumEnclosure)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");   
    EXPECT_THROW(MinEnclosureRule(VIA1, M1, 0.0), std::invalid_argument);
    EXPECT_THROW(MinEnclosureRule(VIA1, M1, -1.0), std::invalid_argument);
}

TEST(MinEnclosureRuleTest, ThrowsOnSameLayerMinimumEnclosure)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    
    EXPECT_THROW(MinEnclosureRule(M1, M1, 0.0), std::invalid_argument);
}

TEST(MinEnclosureRuleTest, ThrowsOnNullLayerMinimumEnclosure)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    
    EXPECT_THROW(MinEnclosureRule(nullptr, M1, 0.0), std::invalid_argument);
}

TEST(MinEnclosureRuleTest, WorksThroughRuleInterface)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");

    // Pointer to a Rule object, but actually holds a MinEnclosureRule
    std::unique_ptr<Rule> rule = std::make_unique<MinEnclosureRule>(VIA1, M1, 4.0);
    Polygon firstPolygon({
        Point(0,0),
        Point(10,0),
        Point(10,10),
        Point(0,10)
        });
    Polygon secondPolygon({
        Point(2,2),
        Point(2,8),
        Point(8,8),
        Point(8,2)
        });

    Shape first(1, M1, std::move(firstPolygon));
    Shape second(2, VIA1, std::move(secondPolygon));
    const std::vector<Shape> shapes{first, second};
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule->check(shapes, spatialIndex);
    EXPECT_EQ(violations.size(), 1);
}
// layer options tests
TEST(MinEnclosureRuleTest, AcceptsFirstAllowedOuterLayer)
{
    LayerRegistry registry;
    const Layer* OD = registry.declare("OD");
    const Layer* CO = registry.declare("CO");
    const Layer* PO = registry.declare("PO");

    Polygon outer({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });

    Polygon inner({
        Point(2, 2),
        Point(4, 2),
        Point(4, 4),
        Point(2, 4)
        });

    Shape outerShape(1, OD, std::move(outer));
    Shape innerShape(2, CO, std::move(inner));

    const std::vector<Shape> shapes{ outerShape, innerShape };
    const LayerSpatialIndex spatialIndex(shapes);

    MinEnclosureRule rule(CO, {EnclosureOption(OD, 1.0), EnclosureOption(PO, 1.0)});

    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}

TEST(MinEnclosureRuleTest, AcceptsSecondAllowedOuterLayer)
{
    LayerRegistry registry;
    const Layer* OD = registry.declare("OD");
    const Layer* CO = registry.declare("CO");
    const Layer* PO = registry.declare("PO");

    Polygon outer({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });

    Polygon inner({
        Point(2, 2),
        Point(4, 2),
        Point(4, 4),
        Point(2, 4)
        });

    Shape outerShape(1, PO, std::move(outer));
    Shape innerShape(2, CO, std::move(inner));

    const std::vector<Shape> shapes{ outerShape, innerShape };
    const LayerSpatialIndex spatialIndex(shapes);

    MinEnclosureRule rule(CO, {EnclosureOption(OD, 1.0), EnclosureOption(PO, 1.0)});

    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}

// Opposite side option tests
TEST(MinEnclosureRuleTest, MinEnclosureWhenFirstOptionFailsButSecondPasses)
{
    LayerRegistry registry;
    const Layer* OD = registry.declare("OD");
    const Layer* CO = registry.declare("CO");
    const Layer* PO = registry.declare("PO");

    Polygon od({
        Point(0, 0),
        Point(5, 0),
        Point(5, 5),
        Point(0, 5)
        });

    Polygon po({
        Point(-5, -5),
        Point(10, -5),
        Point(10, 10),
        Point(-5, 10)
        });

    Polygon inner({
        Point(0.5, 0.5),
        Point(4.5, 0.5),
        Point(4.5, 4.5),
        Point(0.5, 4.5)
        });

    Shape odShape(1, OD, std::move(od));
    Shape poShape(2, PO, std::move(po));
    Shape innerShape(3, CO, std::move(inner));

    const std::vector<Shape> shapes{odShape, poShape, innerShape};
    const LayerSpatialIndex spatialIndex(shapes);

    MinEnclosureRule rule(CO, {EnclosureOption(OD, 1.0), EnclosureOption(PO, 1.0)});

    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}

TEST(MinEnclosureRuleTest, MinEnclosureAcceptsAllSidesEnclosure)
{
    LayerRegistry registry;
    const Layer* OD = registry.declare("OD");
    const Layer* CO = registry.declare("CO");
    const Layer* PO = registry.declare("PO");

    Polygon outer({
        Point(0, 0),
        Point(10, 0),
        Point(10, 10),
        Point(0, 10)
        });

    Polygon inner({
        Point(2, 2),
        Point(8, 2),
        Point(8, 8),
        Point(2, 8)
        });

    Shape outerShape(1, PO, std::move(outer));
    Shape innerShape(2, CO, std::move(inner));

    const std::vector<Shape> shapes{outerShape, innerShape};
    const LayerSpatialIndex spatialIndex(shapes);

    MinEnclosureRule rule(CO, {EnclosureOption(PO, 2.0, 0.0, 4.0)});

    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}

TEST(MinEnclosureRuleTest, MinEnclosureAcceptsFirstOppositeOrientation)
{
    LayerRegistry registry;
    const Layer* OD = registry.declare("OD");
    const Layer* CO = registry.declare("CO");
    const Layer* PO = registry.declare("PO");

    Polygon outer({
        Point(-0.04, 0.0),
        Point(1.04, 0.0),
        Point(1.04, 1.0),
        Point(-0.04, 1.0)
        });

    Polygon inner({
        Point(0.0, 0.0),
        Point(1.0, 0.0),
        Point(1.0, 1.0),
        Point(0.0, 1.0)
        });

    Shape outerShape(1, PO, std::move(outer));
    Shape innerShape(2, CO, std::move(inner));

    const std::vector<Shape> shapes{outerShape, innerShape};

    const LayerSpatialIndex spatialIndex(shapes);

    MinEnclosureRule rule(CO, {EnclosureOption(PO, 0.03, 0.00, 0.04)});

    const auto violations =
        rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}

TEST(MinEnclosureRuleTest, MinEnclosureAcceptsSecondOppositeOrientation)
{
    LayerRegistry registry;
    const Layer* OD = registry.declare("OD");
    const Layer* CO = registry.declare("CO");
    const Layer* PO = registry.declare("PO");

    Polygon outer({
        Point(0.0, -0.04),
        Point(1.0, -0.04),
        Point(1.0, 1.04),
        Point(0.0, 1.04)
        });

    Polygon inner({
        Point(0.0, 0.0),
        Point(1.0, 0.0),
        Point(1.0, 1.0),
        Point(0.0, 1.0)
        });

    Shape outerShape(1, PO, std::move(outer));
    Shape innerShape(2, CO, std::move(inner));

    const std::vector<Shape> shapes{outerShape, innerShape};
    const LayerSpatialIndex spatialIndex(shapes);

    MinEnclosureRule rule(CO,{EnclosureOption(PO, 0.03, 0.00, 0.04)});

    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}

TEST(MinEnclosureRuleTest, MinEnclosureRejectsWhenNoAlternativePasses)
{
    LayerRegistry registry;
    const Layer* OD = registry.declare("OD");
    const Layer* CO = registry.declare("CO");
    const Layer* PO = registry.declare("PO");

    Polygon outer({
        Point(-0.02, 0.0),
        Point(1.02, 0.0),
        Point(1.02, 1.0),
        Point(-0.02, 1.0)
        });

    Polygon inner({
        Point(0.0, 0.0),
        Point(1.0, 0.0),
        Point(1.0, 1.0),
        Point(0.0, 1.0)
        });

    Shape outerShape(1, PO, std::move(outer));
    Shape innerShape(2, CO, std::move(inner));

    const std::vector<Shape> shapes{outerShape, innerShape};
    const LayerSpatialIndex spatialIndex(shapes);

    MinEnclosureRule rule(CO, {EnclosureOption(PO, 0.03, 0.00, 0.04)});

    const auto violations = rule.check(shapes, spatialIndex);

    ASSERT_EQ(violations.size(), 1);

    EXPECT_EQ(violations[0].getType(), ViolationType::Enclosure);
}

TEST(MinEnclosureRuleTest, MinEnclosureAcceptsLaterOuterLayerWhenEarlierOptionFails)
{
    LayerRegistry registry;
    const Layer* OD = registry.declare("OD");
    const Layer* CO = registry.declare("CO");
    const Layer* PO = registry.declare("PO");

    Polygon od({
        Point(-0.02, 0.0),
        Point(1.02, 0.0),
        Point(1.02, 1.0),
        Point(-0.02, 1.0)
        });

    Polygon po({
        Point(-0.04, 0.0),
        Point(1.04, 0.0),
        Point(1.04, 1.0),
        Point(-0.04, 1.0)
        });

    Polygon inner({
        Point(0.0, 0.0),
        Point(1.0, 0.0),
        Point(1.0, 1.0),
        Point(0.0, 1.0)
        });

    Shape odShape(1, OD, std::move(od));
    Shape poShape(2, PO, std::move(po));
    Shape innerShape(3, CO, std::move(inner));

    const std::vector<Shape> shapes{odShape, poShape, innerShape};

    const LayerSpatialIndex spatialIndex(shapes);

    MinEnclosureRule rule(CO, {EnclosureOption(OD, 0.03, 0.00, 0.04), EnclosureOption(PO, 0.03, 0.00, 0.04)});

    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}

TEST(MinEnclosureRuleTest, RejectsBelowOppositePairMinimum)
{
    LayerRegistry registry;
    const Layer* OD = registry.declare("OD");
    const Layer* CO = registry.declare("CO");
    const Layer* PO = registry.declare("PO");

    Polygon outer({
        Point(-0.03, 0.0),
        Point(1.03, 0.0),
        Point(1.03, 1.0),
        Point(-0.03, 1.0)
        });

    Polygon inner({
        Point(0.0, 0.0),
        Point(1.0, 0.0),
        Point(1.0, 1.0),
        Point(0.0, 1.0)
        });

    Shape outerShape(1, PO, std::move(outer));
    Shape innerShape(2, CO, std::move(inner));

    const std::vector<Shape> shapes{outerShape, innerShape};

    const LayerSpatialIndex spatialIndex(shapes);

    MinEnclosureRule rule(CO, {EnclosureOption(PO, 0.04, 0.00, 0.04)});

    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_EQ(violations.size(), 1);
}

TEST(MinEnclosureRuleTest, ReportsAllSidesFailureWhenClosestToPassing)
{
    LayerRegistry registry;
    const Layer* OD = registry.declare("OD");
    const Layer* CO = registry.declare("CO");
    const Layer* PO = registry.declare("PO");

    Polygon outer({
        Point(-0.03, -0.03),
        Point(1.03, -0.03),
        Point(1.03, 1.03),
        Point(-0.03, 1.03)
        });

    Polygon inner({
        Point(0.0, 0.0),
        Point(1.0, 0.0),
        Point(1.0, 1.0),
        Point(0.0, 1.0)
        });

    Shape outerShape(1, PO, std::move(outer));
    Shape innerShape(2, CO, std::move(inner));

    const std::vector<Shape> shapes{ outerShape, innerShape };
    const LayerSpatialIndex spatialIndex(shapes);

    MinEnclosureRule rule(CO, {EnclosureOption(PO, 0.04, 0.00, 0.05)});

    const auto violations = rule.check(shapes, spatialIndex);

    ASSERT_EQ(violations.size(), 1);
    EXPECT_NEAR(violations[0].getActualValue(), 0.03, EPSILON);
    EXPECT_NEAR(violations[0].getRequiredValue(), 0.04, EPSILON);
}

TEST(MinEnclosureRuleTest, ReportsOppositePairFailureWhenClosestToPassing)
{
    LayerRegistry registry;
    const Layer* OD = registry.declare("OD");
    const Layer* CO = registry.declare("CO");
    const Layer* PO = registry.declare("PO");

    Polygon outer({
        Point(-0.03, 0.0),
        Point(1.03, 0.0),
        Point(1.03, 1.0),
        Point(-0.03, 1.0)
        });

    Polygon inner({
        Point(0.0, 0.0),
        Point(1.0, 0.0),
        Point(1.0, 1.0),
        Point(0.0, 1.0)
        });

    Shape outerShape(1, PO, std::move(outer));
    Shape innerShape(2, CO, std::move(inner));

    const std::vector<Shape> shapes{ outerShape, innerShape };
    const LayerSpatialIndex spatialIndex(shapes);

    MinEnclosureRule rule(CO, {EnclosureOption(PO, 0.04, 0.00, 0.05)});

    const auto violations = rule.check(shapes, spatialIndex);

    ASSERT_EQ(violations.size(), 1);
    EXPECT_NEAR(violations[0].getActualValue(), 0.03, EPSILON);
    EXPECT_NEAR(violations[0].getRequiredValue(), 0.05, EPSILON);
}

TEST(MinEnclosureRuleTest, ExactlyEquallToPairwiseMinEnclosurePasses)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");

    Polygon outer({
        Point(0.165, 0.725),
        Point(0.165, 1.305),
        Point(0.265, 1.305),
        Point(0.265, 0.725)
        });

    Polygon inner({
        Point(0.165, 0.925),
        Point(0.265, 0.925),
        Point(0.265, 1.105),
        Point(0.165, 1.105)
        });

    Shape outerShape(1, M1, std::move(outer));
    Shape innerShape(2, VIA1, std::move(inner));

    const std::vector<Shape> shapes{ outerShape, innerShape };
    const LayerSpatialIndex spatialIndex(shapes);

    MinEnclosureRule rule(VIA1, {EnclosureOption(M1, 0.03, 0.00, 0.04)});

    const auto violations = rule.check(shapes, spatialIndex);

    ASSERT_EQ(violations.size(), 0);
}
