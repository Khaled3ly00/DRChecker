#include <gtest/gtest.h>

#include "drcheck/io/JSONLayoutParser.h"
#include "drcheck/geometry/Constants.h"

#include <filesystem>

using namespace drcheck::domain;
using drcheck::io::JSONLayoutParser;
using drcheck::geometry::EPSILON;

TEST(JSONLayoutParserTest, ParsesValidLayoutContainingOneShape) {

    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    const std::filesystem::path path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "basic_layout.json";

    JSONLayoutParser parser;
    const auto shapes = parser.load(path.string(), registry);

    ASSERT_EQ(shapes.size(), 1);
    EXPECT_EQ(shapes[0].getId(), 42);
    EXPECT_EQ(shapes[0].getLayer(), registry.resolve("M1"));
    EXPECT_NEAR(shapes[0].getPolygon().area(), 50.0, EPSILON);
}

TEST(JSONLayoutParserTest, PreservesShapeIdFromJSON) {

    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    const std::filesystem::path path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "basic_layout.json";

    JSONLayoutParser parser;
    const auto shapes = parser.load(path.string(), registry);

    ASSERT_EQ(shapes.size(), 1);
    EXPECT_EQ(shapes[0].getId(), 42);
}

TEST(JSONLayoutParserTest, LayerParserTest) {

    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
	const Layer* M2 = registry.declare("M2");
    const Layer* PO = registry.declare("PO");
    const Layer* OD = registry.declare("OD");
    const Layer* VIA1 = registry.declare("VIA1");

    const std::filesystem::path path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "layer_parser.json";

    JSONLayoutParser parser;
    const auto shapes = parser.load(path.string(), registry);

    ASSERT_EQ(shapes.size(), 5);
    EXPECT_EQ(shapes[0].getLayer(), registry.resolve("M1"));
    EXPECT_EQ(shapes[1].getLayer(), registry.resolve("M2"));
    EXPECT_EQ(shapes[2].getLayer(), registry.resolve("PO"));
    EXPECT_EQ(shapes[3].getLayer(), registry.resolve("OD"));
    EXPECT_EQ(shapes[4].getLayer(), registry.resolve("VIA1"));
}

TEST(JSONLayoutParserTest, InvalidPolygonGeometry) {
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    const std::filesystem::path path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "invalid_polygon.json";

    JSONLayoutParser parser;

    EXPECT_THROW(parser.load(path.string(), registry), std::invalid_argument);
}

TEST(JSONLayoutParserTest, EmptyShapesArray) { 
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    const std::filesystem::path path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "empty_layout.json";

    JSONLayoutParser parser;
    const auto shapes = parser.load(path.string(), registry);

    EXPECT_TRUE(shapes.empty());
}