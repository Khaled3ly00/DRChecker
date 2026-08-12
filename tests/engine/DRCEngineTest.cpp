#include <gtest/gtest.h>

#include "drcheck/engine/DRCEngine.h"

using namespace drcheck::rules;
using namespace drcheck::domain;
using drcheck::geometry::Polygon;
using drcheck::geometry::Point;
using drcheck::engine::DRCEngine;

TEST(DRCEngineTest, CollectsViolationFromOneRule)
{
    Polygon polygon({
        Point(0,0),
        Point(10,0),
        Point(10,2),
        Point(0,2)
        });
    Shape shape(1, Layer::Metal1, std::move(polygon));
    std::vector<Shape> shapes{shape};

    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<MinWidthRule>(Layer::Metal1, 3.0));

    DRCEngine engine;

    const auto violations = engine.run(shapes, rules);

    ASSERT_EQ(violations.size(), 1);
    EXPECT_EQ(violations[0].getType(), ViolationType::MinWidth);
}

TEST(DRCEngineTest, MultipleShapesGeneratingMultipleViolations)
{
    Polygon firstPolygon({ // Violates Metal 1 Min Width
        Point(0,0),
        Point(2,0),
        Point(2,10),
        Point(0,10)
        });
    Shape shape1(1, Layer::Metal1, std::move(firstPolygon));
    Polygon secondPolygon({ // Violates VIA12 Min Width & VIA12 enclosure with Metal 1
        Point(1,1),
        Point(1.5,1),
        Point(1.5,1.5),
        Point(1,1.5)
        });
    Shape shape2(2, Layer::Via12, std::move(secondPolygon));
    std::vector<Shape> shapes{shape1, shape2};

    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<MinWidthRule>(Layer::Metal1, 3.0));
    rules.push_back(std::make_unique<MinWidthRule>(Layer::Via12, 1.0));
    rules.push_back(std::make_unique<MinEnclosureRule>(Layer::Via12, Layer::Metal1, 1.0));

    DRCEngine engine;

    const auto violations = engine.run(shapes, rules);

    EXPECT_EQ(violations.size(), 3);
}

TEST(DRCEngineTest, MultipleShapesGeneratingMultipleViolations1)
{
    Polygon firstPolygon({ // Violates Metal 1 Min Width
        Point(0,0),
        Point(2,0),
        Point(2,10),
        Point(0,10)
        });
    Shape shape1(1, Layer::Metal1, std::move(firstPolygon));
    Polygon secondPolygon({ // Violates VIA12 Min Width & VIA12 enclosure with Metal 1
        Point(1,1),
        Point(1.5,1),
        Point(1.5,1.5),
        Point(1,1.5)
        });
    Shape shape2(2, Layer::Via12, std::move(secondPolygon));
    Polygon ThirdPolygon({ // Violates Metal 1 Spacing (with first polygon)
        Point(3,0),
        Point(6,0),
        Point(6,10),
        Point(3,10)
        });
    Shape shape3(3, Layer::Metal1, std::move(ThirdPolygon));
    std::vector<Shape> shapes{shape1, shape2, shape3};

    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<MinWidthRule>(Layer::Metal1, 3.0));
    rules.push_back(std::make_unique<MinWidthRule>(Layer::Via12, 1.0));
    rules.push_back(std::make_unique<MinEnclosureRule>(Layer::Via12, Layer::Metal1, 1.0));
    rules.push_back(std::make_unique<MinSpacingRule>(Layer::Metal1, 2.0));

    DRCEngine engine;

    const auto violations = engine.run(shapes, rules);

    EXPECT_EQ(violations.size(), 4);
}

TEST(DRCEngineTest, MultipleViolationsFromOneRule) {
    Polygon firstPolygon({ // Violates Metal 1 Min Width
        Point(0,0),
        Point(2,0),
        Point(2,10),
        Point(0,10)
        });
    Shape shape1(1, Layer::Metal1, std::move(firstPolygon));
    Polygon secondPolygon({ // Violates Metal 1 Min Width
        Point(3,0),
        Point(5.5,0),
        Point(5.5,8),
        Point(3,8)
        });
    Shape shape2(2, Layer::Metal1, std::move(secondPolygon));
    std::vector<Shape> shapes{shape1, shape2};

    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<MinWidthRule>(Layer::Metal1, 3.0));

    DRCEngine engine;
    const auto violations = engine.run(shapes, rules);

    EXPECT_EQ(violations.size(), 2);
}

TEST(DRCEngineTest, ReturnsNoViolationsWhenNoRules) {
    Polygon polygon({
        Point(0,0),
        Point(10,0),
        Point(10,2),
        Point(0,2)
        });
    Shape shape(1, Layer::Metal1, std::move(polygon));
    std::vector<Shape> shapes{shape};

    std::vector<std::unique_ptr<Rule>> rules;

    DRCEngine engine;
    const auto violations = engine.run(shapes, rules);

    EXPECT_TRUE(violations.empty());
}

TEST(DRCEngineTest, ReturnsNoViolationsWhenNoShapes) {
    std::vector<Shape> shapes;

    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<MinWidthRule>(Layer::Metal1, 3.0));
    rules.push_back(std::make_unique<MinWidthRule>(Layer::Via12, 1.0));
    rules.push_back(std::make_unique<MinEnclosureRule>(Layer::Via12, Layer::Metal1, 1.0));

    DRCEngine engine;
    const auto violations = engine.run(shapes, rules);

    EXPECT_TRUE(violations.empty());
}

TEST(DRCEngineTest, IgnoresNullRules) {
    Polygon polygon({
        Point(0,0),
        Point(10,0),
        Point(10,2),
        Point(0,2)
        });
    Shape shape(1, Layer::Metal1, std::move(polygon));
    std::vector<Shape> shapes{shape};

    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(nullptr);

    DRCEngine engine;
    const auto violations = engine.run(shapes, rules);

    EXPECT_TRUE(violations.empty());
}