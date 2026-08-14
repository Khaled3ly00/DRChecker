#include <gtest/gtest.h>

#include "drcheck/rules/MinSpacingRule.h"
#include "drcheck/geometry/Constants.h"

#include <memory>

using drcheck::geometry::EPSILON;
using drcheck::rules::MinSpacingRule;
using drcheck::geometry::Polygon;
using drcheck::geometry::Point;
using namespace drcheck::domain;
using drcheck::rules::Rule;

TEST(MinSpacingRuleTest, DetectsSpacingViolation)
{
    Polygon firstPolygon({
        Point(0,0),
        Point(5,0),
        Point(5,5),
        Point(0,5)
        });
    Polygon secondPolygon({
        Point(7,0),
        Point(12,0),
        Point(12,5),
        Point(7,5)
        });

    Shape first(1, Layer::Metal1, std::move(firstPolygon));
    Shape second(2, Layer::Metal1, std::move(secondPolygon));
    MinSpacingRule rule(Layer::Metal1, 3.0);
    const std::vector<Shape> shapes{first, second};
    const auto violations = rule.check(shapes);

    ASSERT_EQ(violations.size(), 1);
    ASSERT_EQ(violations[0].getShapeIds().size(), 2);
    EXPECT_EQ(violations[0].getShapeIds()[0], 1);
    EXPECT_EQ(violations[0].getShapeIds()[1], 2);
	EXPECT_NEAR(violations[0].getActualValue(), 2.0, EPSILON);
	EXPECT_NEAR(violations[0].getRequiredValue(), 3.0, EPSILON);
}

TEST(MinSpacingRuleTest, AcceptsShapesMeetingMinimumSpacing)
{
	Polygon firstPolygon({
		Point(0,0),
		Point(5,0),
		Point(5,5),
		Point(0,5)
		});
	Polygon secondPolygon({
		Point(8,0),
		Point(13,0),
		Point(13,5),
		Point(8,5)
		});
	Shape first(3, Layer::Metal1, std::move(firstPolygon));
	Shape second(4, Layer::Metal1, std::move(secondPolygon));
	MinSpacingRule rule(Layer::Metal1, 3.0);
	const std::vector<Shape> shapes{first, second};
	const auto violations = rule.check(shapes);
	EXPECT_TRUE(violations.empty());
}

TEST(MinSpacingRuleTest, AcceptsShapesLargerThanMinimumSpacing)
{
	Polygon firstPolygon({
		Point(0,0),
		Point(5,0),
		Point(5,5),
		Point(0,5)
		});
	Polygon secondPolygon({
		Point(8,0),
		Point(13,0),
		Point(13,5),
		Point(8,5)
		});
	Shape first(3, Layer::Metal1, std::move(firstPolygon));
	Shape second(4, Layer::Metal1, std::move(secondPolygon));
	MinSpacingRule rule(Layer::Metal1, 2.0);
	const std::vector<Shape> shapes{ first, second };
	const auto violations = rule.check(shapes);
	EXPECT_TRUE(violations.empty());
}

TEST(MinSpacingRuleTest, IgnoresShapesOnOtherLayers)
{
	Polygon firstPolygon({
		Point(0,0),
		Point(5,0),
		Point(5,5),
		Point(0,5)
		});
	Polygon secondPolygon({
		Point(7,0),
		Point(12,0),
		Point(12,5),
		Point(7,5)
		});
	Shape first(5, Layer::Metal1, std::move(firstPolygon));
	Shape second(6, Layer::Metal2, std::move(secondPolygon));
	MinSpacingRule rule(Layer::Metal1, 3.0);
	const std::vector<Shape> shapes{ first, second };
	const auto violations = rule.check(shapes);
	EXPECT_TRUE(violations.empty());
}

TEST(MinSpacingRuleTest, ThrowsOnNonPositiveMinimumSpacing)
{
	EXPECT_THROW(MinSpacingRule(Layer::Metal1, 0.0), std::invalid_argument);
	EXPECT_THROW(MinSpacingRule(Layer::Metal1, -1.0), std::invalid_argument);
}

TEST(MinSpacingRuleTest, MultipleShapesAgainstMinSpacing)
{
	Polygon polygon1({
		Point(0,0),
		Point(5,0),
		Point(5,5),
		Point(0,5)
		});
	Shape shape1(1, Layer::Metal1, std::move(polygon1));
	Polygon polygon2({
		Point(7,0),
		Point(12,0),
		Point(12,5),
		Point(7,5)
		});
	Shape shape2(2, Layer::Metal1, std::move(polygon2));
	Polygon polygon3({
		Point(15.0, 0.0),
		Point(20.0, 0.0),
		Point(20.0, 5.0),
		Point(15.0, 5.0)
		});
	Shape shape3(3, Layer::Metal1, std::move(polygon3));
	MinSpacingRule rule(Layer::Metal1, 3.0);
	const auto violations = rule.check({ shape1, shape2, shape3 });
	ASSERT_EQ(violations.size(), 1);
	EXPECT_EQ(violations[0].getShapeIds()[0], 1);
	EXPECT_EQ(violations[0].getShapeIds()[1], 2);
}

TEST(MinSpacingRule, IntersectingShapesAgainstMinSpacing) {
	Polygon polygon1({
		Point(0,0),
		Point(5,0),
		Point(5,5),
		Point(0,5)
		});
	Shape shape1(1, Layer::Metal2, std::move(polygon1));
	Polygon polygon2({
		Point(4,0),
		Point(9,0),
		Point(9,5),
		Point(4,5)
		});
	Shape shape2(2, Layer::Metal2, std::move(polygon2));
	MinSpacingRule rule(Layer::Metal2, 3.0);
	const auto violations = rule.check({shape1, shape2});
	ASSERT_EQ(violations.size(), 1);
	EXPECT_EQ(violations[0].getShapeIds()[0], 1);
	EXPECT_EQ(violations[0].getShapeIds()[1], 2);
}

TEST(MinSpacingRule, LShapesAgainstMinSpacing) {
	Polygon polygon1({
		Point(0.0, 0.0),
		Point(4.0, 0.0),
		Point(4.0, 5.0),
		Point(10.0, 5.0),
		Point(10.0, 8.0),
		Point(0.0, 8.0)
		});
	Shape shape1(1, Layer::Metal2, std::move(polygon1));
	Polygon polygon2({
		Point(11.0, 0.0),
		Point(11.0, 5.0),
		Point(15.0, 5.0),
		Point(15.0, 0.0)
		});
	Shape shape2(2, Layer::Metal2, std::move(polygon2));
	MinSpacingRule rule(Layer::Metal2, 1.0);
	const auto violations = rule.check({ shape1, shape2 });
	EXPECT_TRUE(violations.empty());
}

// Testing Rule Base Class Polymorphism with MinSpacingRule
// Test that MinSpacingRule can be used through the Rule interface(Abstract Class)
TEST(MinSpacingRuleTest, WorksThroughRuleInterface)
{
	// Pointer to a Rule object, but actually holds a MinSpacingRule
	std::unique_ptr<Rule> rule = std::make_unique<MinSpacingRule>(Layer::Metal1, 4.0);
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
	const auto violations = rule->check({shape1, shape2});
	ASSERT_EQ(violations.size(), 1);
	EXPECT_EQ(violations[0].getShapeIds()[0], 1);
	EXPECT_EQ(violations[0].getShapeIds()[1], 2);
}