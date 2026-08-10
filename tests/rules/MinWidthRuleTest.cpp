#include <gtest/gtest.h>

#include "drcheck/rules/MinWidthRule.h"

#include <memory>

using drcheck::rules::MinWidthRule;
using drcheck::geometry::Polygon;
using drcheck::geometry::Point;
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
    const auto violations = rule.check({shape});
    ASSERT_EQ(violations.size(), 1);
    EXPECT_EQ(violations[0].getType(), ViolationType::MinWidth);
    EXPECT_EQ(violations[0].getShapeIds()[0], 1);
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

    EXPECT_TRUE(rule.check({shape}).empty());
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
	EXPECT_TRUE(rule.check({shape}).empty());
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
    EXPECT_TRUE(rule.check({shape}).empty());
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
	const auto violations = rule.check({ shape1, shape2, shape3 });
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
    const auto violations = rule.check({shape});
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
    const auto violations = rule->check({shape1, shape2});
    EXPECT_EQ(violations.size(), 1);
}