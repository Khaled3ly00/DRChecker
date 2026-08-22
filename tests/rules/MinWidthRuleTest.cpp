#include <gtest/gtest.h>

#include "drcheck/rules/MinWidthRule.h"
#include "drcheck/geometry/Segment.h"
#include "drcheck/geometry/Constants.h"

#include <memory>

using drcheck::geometry::EPSILON;
using drcheck::rules::MinWidthRule;
using drcheck::geometry::Polygon;
using drcheck::geometry::Point;
using drcheck::geometry::Segment;
using namespace drcheck::domain;
using drcheck::rules::Rule;

TEST(MinWidthRuleTest, DetectsMinWidthViolation)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 3.0),
        Point(0.0, 3.0)
    });
    Shape shape(1,Layer::Metal1,std::move(polygon));
    MinWidthRule rule(Layer::Metal1,4.0);
    const std::vector<Shape> shapes{ shape };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);
    ASSERT_EQ(violations.size(), 1);
    EXPECT_EQ(violations[0].getType(), ViolationType::MinWidth);
    EXPECT_EQ(violations[0].getShapeIds()[0], 1);
    EXPECT_NEAR(violations[0].getActualValue(), 3.0, EPSILON);
    EXPECT_NEAR(violations[0].getRequiredValue(), 4.0, EPSILON);
    // Marker tests
    ASSERT_TRUE(violations[0].getMarker().has_value());
    const auto marker = violations[0].getMarker().value();
    ASSERT_TRUE(marker.firstEdgeIndex.has_value());
    ASSERT_TRUE(marker.secondEdgeIndex.has_value());
    const auto edges = shape.getPolygon().getEdges();
    ASSERT_TRUE(marker.firstPoint.has_value());
    ASSERT_TRUE(marker.secondPoint.has_value());
    EXPECT_TRUE(edges[marker.firstEdgeIndex.value()].contains(marker.firstPoint.value()));
    EXPECT_TRUE(edges[marker.secondEdgeIndex.value()].contains(marker.secondPoint.value()));
    // Distance can be retrieved as we calculate distance between 2 points
    EXPECT_NEAR(Point::vectorBetween(marker.firstPoint.value(), marker.secondPoint.value()).length(), 3.0, EPSILON);
}

TEST(MinWidthRuleTest, AcceptsShapeMeetingMinWidth)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 5.0),
        Point(0.0, 5.0)
        });

    Shape shape(1, Layer::Metal1, std::move(polygon));

    MinWidthRule rule(Layer::Metal1, 4.0);
    const std::vector<Shape> shapes{ shape };
    LayerSpatialIndex spatialIndex(shapes);
    EXPECT_TRUE(rule.check(shapes, spatialIndex).empty());
}

TEST(MinWidthRuleTest, AcceptsShapeExactlyAtMinimumWidth)
{
	Polygon polygon({
		Point(0.0, 0.0),
		Point(10.0, 0.0),
		Point(10.0, 4.0),
		Point(0.0, 4.0)
		});
	Shape shape(1, Layer::Metal1, std::move(polygon));
	MinWidthRule rule(Layer::Metal1, 4.0);
    const std::vector<Shape> shapes{ shape };
    LayerSpatialIndex spatialIndex(shapes);
    EXPECT_TRUE(rule.check(shapes, spatialIndex).empty());
}

TEST(MinWidthRuleTest, IgnoresShapesOnOtherLayers)
{
    Polygon polygon({
        Point(0,0),
        Point(10,0),
        Point(10,5),
        Point(0,5)
        });
    Shape shape(7, Layer::Metal2, std::move(polygon));
    MinWidthRule rule(Layer::Metal1, 4.0);
    const std::vector<Shape> shapes{ shape };
    LayerSpatialIndex spatialIndex(shapes);
    EXPECT_TRUE(rule.check(shapes, spatialIndex).empty());
}

TEST(MinWidthRuleTest, ThrowsOnNonPositiveMinimumWidth)
{
	EXPECT_THROW(MinWidthRule(Layer::Metal1, 0.0), std::invalid_argument);
	EXPECT_THROW(MinWidthRule(Layer::Metal1, -1.0), std::invalid_argument);
}

TEST(MinWidthRuleTest, MultipleShapesAgainstMinWidth)
{
	Polygon polygon1({
		Point(0,0),
		Point(10,0),
		Point(10,3),
		Point(0,3)
		});
	Shape shape1(1, Layer::Metal1, std::move(polygon1));
	Polygon polygon2({
		Point(0,0),
		Point(10,0),
		Point(10,5),
		Point(0,5)
		});
	Shape shape2(2, Layer::Metal1, std::move(polygon2));
	Polygon polygon3({
	Point(0.0, 0.0),
	Point(10.0, 0.0),
	Point(10.0, 4.0),
	Point(0.0, 4.0)
		});
	Shape shape3(3, Layer::Metal1, std::move(polygon3));
	MinWidthRule rule(Layer::Metal1, 4.0);
    const std::vector<Shape> shapes{ shape1, shape2, shape3 };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);
    ASSERT_EQ(violations.size(), 1);
	EXPECT_EQ(violations[0].getShapeIds()[0], 1);
}

TEST(MinWidthRuleTest, MinimumWidthRuleOfHShape) {
    Polygon polygon({
        Point(0.0, 0.0),
        Point(6.0, 0.0),
        Point(6.0, 2.0),
        Point(4.0, 2.0),
        Point(4.0, 4.0),
        Point(6.0, 4.0),
        Point(6.0, 6.0),
        Point(0.0, 6.0),
        Point(0.0, 4.0),
        Point(3.0, 4.0),
        Point(3.0, 2.0),
        Point(0.0, 2.0)
        });
	Shape shape(10, Layer::Metal1, std::move(polygon));
	MinWidthRule rule(Layer::Metal1, 1.5);
    const std::vector<Shape> shapes{ shape };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);
    ASSERT_EQ(violations.size(), 1);
    EXPECT_EQ(violations[0].getType(), ViolationType::MinWidth);
    EXPECT_EQ(violations[0].getShapeIds()[0], 10);
}

// Testing Rule Base Class Polymorphism with MinWidthRule
// Test that MinWidthRule can be used through the Rule interface(Abstract Class)
TEST(MinWidthRuleTest, WorksThroughRuleInterface)
{
	// Pointer to a Rule object, but actually holds a MinWidthRule
    std::unique_ptr<Rule> rule = std::make_unique<MinWidthRule> (Layer::Metal1, 4.0);
    Polygon polygon1({
        Point(0,0),
        Point(10,0),
        Point(10,3),
        Point(0,3)
        });
    Shape shape1(1, Layer::Metal1, std::move(polygon1));
    Polygon polygon2({
        Point(0,0),
        Point(10,0),
        Point(10,5),
        Point(0,5)
        });
    Shape shape2(2, Layer::Metal1, std::move(polygon2));
    const std::vector<Shape> shapes{ shape1, shape2 };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule->check(shapes, spatialIndex);
    EXPECT_EQ(violations.size(), 1);
}