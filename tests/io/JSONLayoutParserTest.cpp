#include <gtest/gtest.h>

#include "drcheck/io/JSONLayoutParser.h"
#include "drcheck/geometry/Constants.h"

#include <filesystem>

using namespace drcheck::domain;
using drcheck::io::JSONLayoutParser;
using drcheck::geometry::EPSILON;

TEST(JSONLayoutParserTest, ValidJSONLayoutContainingOneShapeWithNoViolation) {
    const std::filesystem::path path = std::filesystem::path(DRCHECK_SOURCE_DIR) /"examples"/"basic_layout.json";
    JSONLayoutParser parser;
    const auto shapes = parser.load(path.string());

    ASSERT_EQ(shapes.size(), 1);
    EXPECT_EQ(shapes[0].getId(), 0);
    EXPECT_EQ(shapes[0].getLayer(),Layer::Metal1);
    EXPECT_NEAR(shapes[0].getPolygon().area(), 50.0, EPSILON);
}

TEST(JSONLayoutParserTest, LayerParserTest) {
    const std::filesystem::path path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "layer_parser.json";
    JSONLayoutParser parser;
    const auto shapes = parser.load(path.string());

    ASSERT_EQ(shapes.size(), 5);
    EXPECT_EQ(shapes[0].getLayer(), Layer::Metal1);
    EXPECT_EQ(shapes[1].getLayer(), Layer::Metal2);
    EXPECT_EQ(shapes[2].getLayer(), Layer::Poly);
    EXPECT_EQ(shapes[3].getLayer(), Layer::Diffusion);
    EXPECT_EQ(shapes[4].getLayer(), Layer::Via12);
}

TEST(JSONLayoutParserTest, InvalidPolygonGeometry) {
    // Self Intersecting Polygon
    const std::filesystem::path path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "invalid_polygon.json";
    JSONLayoutParser parser;

    EXPECT_THROW(parser.load(path.string()), std::invalid_argument);
}

TEST(JSONLayoutParserTest, EmptyShapesArray) {
    const std::filesystem::path path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "empty_layout.json";
    JSONLayoutParser parser;
    const auto shapes = parser.load(path.string());

    EXPECT_TRUE(shapes.empty());
}