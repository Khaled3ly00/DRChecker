#include <gtest/gtest.h>

#include "drcheck/rules/MinEnclosureRule.h"
#include "drcheck/geometry/Constants.h"

#include <memory>

using drcheck::geometry::EPSILON;
using drcheck::rules::Rule;
using drcheck::rules::MinEnclosureRule;
using drcheck::geometry::Polygon;
using drcheck::geometry::Point;
using namespace drcheck::domain;

// MinEnclosureRule(innerLayer, outerLayer, minimumEnclosure)

TEST(MinEnclosureRuleTest, AcceptsExactMinEnclosure)
{
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

    Shape first(1, Layer::Metal1, std::move(firstPolygon));
    Shape second(2, Layer::Via12, std::move(secondPolygon));
    MinEnclosureRule rule(Layer::Via12, Layer::Metal1, 2.0);
    const std::vector<Shape> shapes{first, second};
    // Creates QuadTree
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}

TEST(MinEnclosureRuleTest, AcceptsMoreThanMinEnclosure)
{
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

    Shape first(1, Layer::Metal1, std::move(firstPolygon));
    Shape second(2, Layer::Via12, std::move(secondPolygon));
    MinEnclosureRule rule(Layer::Via12, Layer::Metal1, 2.0);
    const std::vector<Shape> shapes{ first, second };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}

TEST(MinEnclosureRuleTest, RejectsLessThanMinEnclosure)
{
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

    Shape first(1, Layer::Metal1, std::move(firstPolygon));
    Shape second(2, Layer::Via12, std::move(secondPolygon));
    MinEnclosureRule rule(Layer::Via12, Layer::Metal1, 3.0);
    const std::vector<Shape> shapes{first, second};
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
    EXPECT_EQ(marker.firstEdgeIndex, 0);
    EXPECT_EQ(marker.secondEdgeIndex, 3);
    EXPECT_NEAR(Point::vectorBetween(marker.firstPoint, marker.secondPoint).length(), 2.0, EPSILON);
    const auto innerEdges = second.getPolygon().getEdges();
    const auto outerEdges = first.getPolygon().getEdges();
    ASSERT_TRUE(marker.firstEdgeIndex.has_value());
    ASSERT_TRUE(marker.secondEdgeIndex.has_value());
    EXPECT_TRUE(innerEdges[marker.firstEdgeIndex.value()].contains(marker.firstPoint));
    EXPECT_TRUE(outerEdges[marker.secondEdgeIndex.value()].contains(marker.secondPoint));

}

TEST(MinEnclosureRuleTest, IntersectingPolygonsMinEnclosure) // Intersecting polygon should return violation as enclosure distance is zero
{
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

    Shape first(1, Layer::Via12, std::move(firstPolygon));
    Shape second(2, Layer::Metal1, std::move(secondPolygon));
    MinEnclosureRule rule(Layer::Via12, Layer::Metal1, 1.0);
    const std::vector<Shape> shapes{first, second};
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_EQ(violations.size(), 1);    
    // Marker tests
    // Currently intersecting polygons return no foundContainingOuter
    ASSERT_FALSE(violations[0].getMarker().has_value());
}

TEST(MinEnclosureRuleTest, TouchingInternallyPolygonsMinEnclosure) // Touching Internally polygon should return violation as enclosure distance is zero
{
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

    Shape first(1, Layer::Metal1, std::move(firstPolygon));
    Shape second(2, Layer::Via12, std::move(secondPolygon));
    MinEnclosureRule rule(Layer::Via12, Layer::Metal1, 1.0);
    const std::vector<Shape> shapes{ first, second };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_EQ(violations.size(), 1);
}

TEST(MinEnclosureRuleTest, InnerCompletelyOutsideMinEnclosure) {
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

    Shape first(1, Layer::Via12, std::move(firstPolygon));
    Shape second(2, Layer::Metal1, std::move(secondPolygon));
    MinEnclosureRule rule(Layer::Via12, Layer::Metal1, 2.0);
    const std::vector<Shape> shapes{ first, second };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_EQ(violations.size(), 1);
}


TEST(MinEnclosureRuleTest, InnerCompletelyInsideConcavePolygonMinEnclosure) {
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

    Shape first(1, Layer::Via12, std::move(firstPolygon));
    Shape second(2, Layer::Metal1, std::move(secondPolygon));
    MinEnclosureRule rule(Layer::Via12, Layer::Metal1, 2.0);
    const std::vector<Shape> shapes{ first, second };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}

TEST(MinEnclosureRuleTest, InnerIntersectsOuterConcavePolygonMinEnclosure) {
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

    Shape first(1, Layer::Via12, std::move(firstPolygon));
    Shape second(2, Layer::Metal1, std::move(secondPolygon));
    MinEnclosureRule rule(Layer::Via12, Layer::Metal1, 3.0);
    const std::vector<Shape> shapes{ first, second };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_EQ(violations.size(), 1);
}
// If the rule checks for via12 enclosure within Metal 1
// But there's only 2 shapes: metal 2 and via12
// Then if rule is checked a violation appears as there's no via12 enclosed within Metal1
TEST(MinEnclosureRuleTest,NoMatchingOuterLayerMinEnclosure)
{
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

    Shape first(1, Layer::Metal2, std::move(firstPolygon));
    Shape second(2, Layer::Via12, std::move(secondPolygon));
    MinEnclosureRule rule(Layer::Via12, Layer::Metal1, 2.0);
    const std::vector<Shape> shapes{ first, second };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_EQ(violations.size(), 1);
    EXPECT_FALSE(violations[0].getMarker().has_value());
}
// Multiple outer candidates, one valid. Have two Metal1 shapes:
// one does not contain the Via12, while another provides sufficient enclosure.
TEST(MinEnclosureRuleTest, MultipleOuterLayerPolygonsMinEnclosure)
{
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

    Shape first(1, Layer::Metal1, std::move(firstPolygon));
    Shape second(2, Layer::Metal1, std::move(secondPolygon));
    Shape third(3, Layer::Via12, std::move(thirdPolygon));
    MinEnclosureRule rule(Layer::Via12, Layer::Metal1, 2.0);
    const std::vector<Shape> shapes{first, second, third};
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    EXPECT_TRUE(violations.empty());
}
// Multiple inner shapes. For example, 
// one Via12 correctly enclosed and another Via12 insufficiently enclosed. 
// Expected result: exactly one violation
TEST(MinEnclosureRuleTest, MultipleInnerLayerPolygonsMinEnclosure)
{
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
    Shape first(1, Layer::Metal1, std::move(firstPolygon));
    Shape second(2, Layer::Via12, std::move(secondPolygon));
    Shape third(3, Layer::Via12, std::move(thirdPolygon));
    MinEnclosureRule rule(Layer::Via12, Layer::Metal1, 2.0);
    const std::vector<Shape> shapes{ first, second, third };
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule.check(shapes, spatialIndex);

    ASSERT_EQ(violations.size(), 1);
    EXPECT_EQ(violations[0].getShapeIds()[0], 3);
}

TEST(MinEnclosureRuleTest, ThrowsOnNonPositiveMinimumEnclosure)
{
    EXPECT_THROW(MinEnclosureRule(Layer::Via12, Layer::Metal1, 0.0), std::invalid_argument);
    EXPECT_THROW(MinEnclosureRule(Layer::Via12, Layer::Metal1, -1.0), std::invalid_argument);
}

TEST(MinEnclosureRuleTest, ThrowsOnSameLayerMinimumEnclosure)
{
    EXPECT_THROW(MinEnclosureRule(Layer::Metal1, Layer::Metal1, 0.0), std::invalid_argument);
}

TEST(MinEnclosureRuleTest, WorksThroughRuleInterface)
{
    // Pointer to a Rule object, but actually holds a MinEnclosureRule
    std::unique_ptr<Rule> rule = std::make_unique<MinEnclosureRule>(Layer::Via12, Layer::Metal1, 4.0);
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

    Shape first(1, Layer::Metal1, std::move(firstPolygon));
    Shape second(2, Layer::Via12, std::move(secondPolygon));
    const std::vector<Shape> shapes{first, second};
    LayerSpatialIndex spatialIndex(shapes);
    const auto violations = rule->check(shapes, spatialIndex);
    EXPECT_EQ(violations.size(), 1);
}