#include <gtest/gtest.h>
#include <gdstk/gdstk.hpp>
#include <filesystem>
#include <string>

#include "drcheck/io/GDSLayoutParser.h"
#include "drcheck/domain/LayerRegistry.h"
#include "drcheck/domain/Layer.h"
#include "drcheck/geometry/BoundingBox.h"
#include "drcheck/geometry/Constants.h"

using drcheck::io::GDSLayoutParser;
using drcheck::domain::LayerRegistry;
using drcheck::domain::Layer;
using drcheck::geometry::EPSILON;

TEST(GDSLayoutParserTest, WriteGDSWithOneRectangleThenParseIt) {
    // Create GDS containing rectangle
    const std::string filePath = "gdstk_metadata_test.gds";

    gdstk::Library library{};
    library.init("TEST", 1e-6, 1e-9);

    auto* top = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));

    top->init("TOP");

    auto* rectangle = static_cast<gdstk::Polygon*>(gdstk::allocate_clear(sizeof(gdstk::Polygon)));

    *rectangle = gdstk::rectangle(
        gdstk::Vec2{ 1.0, 2.0 },
        gdstk::Vec2{ 5.0, 6.0 },
        gdstk::make_tag(15, 0)
    );

    top->polygon_array.append(rectangle);
    library.cell_array.append(top);
    // Write GDS
    ASSERT_EQ(library.write_gds(filePath.c_str(), 0, nullptr), gdstk::ErrorCode::NoError);

    // Create Layer Registry before reading GDS
    LayerRegistry registry;
    const Layer* m1 = registry.declare("M1");
    registry.mapGDS(m1, 15, 0);
    // Read GDS
    const auto shapes = GDSLayoutParser::load(filePath, registry);

    ASSERT_EQ(shapes.size(), 1);

    EXPECT_EQ(shapes[0].getLayer(), m1);
    EXPECT_EQ(shapes[0].getId(), 1);

    const auto bounds =shapes[0].getPolygon().getBoundingBox();

    EXPECT_DOUBLE_EQ(bounds.getMinX(), 1.0);
    EXPECT_DOUBLE_EQ(bounds.getMinY(), 2.0);
    EXPECT_DOUBLE_EQ(bounds.getMaxX(), 5.0);
    EXPECT_DOUBLE_EQ(bounds.getMaxY(), 6.0);
}


TEST(GDSLayoutParserTest, WriteGDSWithOneRectangleWithUnmappedLayerThenParseIt) {
    // Create GDS containing rectangle
    const std::string filePath = "gdstk_metadata_test.gds";

    gdstk::Library library{};
    library.init("TEST", 1e-6, 1e-9);

    auto* top = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));

    top->init("TOP");

    auto* rectangle = static_cast<gdstk::Polygon*>(gdstk::allocate_clear(sizeof(gdstk::Polygon)));

    *rectangle = gdstk::rectangle(
        gdstk::Vec2{ 1.0, 2.0 },
        gdstk::Vec2{ 5.0, 6.0 },
        gdstk::make_tag(99, 7)
    );

    top->polygon_array.append(rectangle);
    library.cell_array.append(top);
    // Write GDS
    ASSERT_EQ(library.write_gds(filePath.c_str(), 0, nullptr), gdstk::ErrorCode::NoError);

    // Create Layer Registry before reading GDS
    LayerRegistry registry;
    const Layer* m1 = registry.declare("M1");
    registry.mapGDS(m1, 15, 0);
    // Read GDS
    EXPECT_THROW(GDSLayoutParser::load(filePath, registry), std::invalid_argument);
}

TEST(GDSLayoutParserTest, RejectsHierarchicalLayout)
{
    const std::string filePath = "gds_hierarchy_test.gds";

    gdstk::Library library{};
    library.init("TEST_LIBRARY", 1e-6, 1e-9);

    auto* top = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    top->init("TOP");

    auto* child = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    child->init("CHILD");

    auto* rectangle = static_cast<gdstk::Polygon*>(gdstk::allocate_clear(sizeof(gdstk::Polygon)));

    *rectangle = gdstk::rectangle(
        gdstk::Vec2{ 0.0, 0.0 },
        gdstk::Vec2{ 10.0, 10.0 },
        gdstk::make_tag(15, 0)
    );

    child->polygon_array.append(rectangle);

    auto* reference = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));

    reference->init(child);
    top->reference_array.append(reference);

    library.cell_array.append(top);
    library.cell_array.append(child);

    ASSERT_EQ(library.write_gds(filePath.c_str(), 0, nullptr), gdstk::ErrorCode::NoError);

    library.free_all();

    LayerRegistry registry;

    const Layer* m1 = registry.declare("M1");

    registry.mapGDS(m1, 15, 0);

    EXPECT_THROW(GDSLayoutParser::load(filePath, registry), std::invalid_argument);

    std::filesystem::remove(filePath);
}

TEST(GDSLayoutParserTest, RejectsMultipleTopLevelCells)
{
    const std::string filePath = "gds_multiple_top_cells_test.gds";

    gdstk::Library library{};
    library.init("TEST_LIBRARY", 1e-6, 1e-9);

    auto* topA = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    topA->init("TOP_A");

    auto* topB = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    topB->init("TOP_B");

    library.cell_array.append(topA);
    library.cell_array.append(topB);

    ASSERT_EQ(library.write_gds(filePath.c_str(), 0, nullptr), gdstk::ErrorCode::NoError);

    library.free_all();

    LayerRegistry registry;

    EXPECT_THROW(GDSLayoutParser::load(filePath, registry), std::invalid_argument);

    std::filesystem::remove(filePath);
}

TEST(GDSLayoutParserTest, ImportsStraightPath)
{
    const std::string filePath = "gds_path_test.gds";

    gdstk::Library library{};
    library.init("TEST_LIBRARY", 1e-6, 1e-9);

    auto* top = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    top->init("TOP");

    auto* flushEndPath = static_cast<gdstk::FlexPath*>(gdstk::allocate_clear(sizeof(gdstk::FlexPath)));
    flushEndPath->init(
        gdstk::Vec2{ 0.0, 0.0 },
        1, 2.0, 0.0, 1e-3,
        gdstk::make_tag(15, 0));
    flushEndPath->simple_path = true;
    flushEndPath->elements[0].end_type = gdstk::EndType::Flush;
    flushEndPath->segment(
        gdstk::Vec2{ 10.0, 0.0 },
        nullptr, nullptr, false);

    auto* extendedEndPath = static_cast<gdstk::FlexPath*>(gdstk::allocate_clear(sizeof(gdstk::FlexPath)));
    extendedEndPath->init(
        gdstk::Vec2{ 0.0, 0.0 },
        1, 2.0, 0.0, 1e-3,
        gdstk::make_tag(15, 0));
    extendedEndPath->simple_path = true;
    extendedEndPath->elements[0].end_type = gdstk::EndType::Extended;
    extendedEndPath->elements[0].end_extensions =
        gdstk::Vec2{ 2.0, 3.0 };
    extendedEndPath->segment(
        gdstk::Vec2{ 10.0, 0.0 },
        nullptr, nullptr, false);


    auto* roundEndPath = static_cast<gdstk::FlexPath*>(gdstk::allocate_clear(sizeof(gdstk::FlexPath)));
    roundEndPath->init(
        gdstk::Vec2{ 0.0, 0.0 },
        1, 2.0, 0.0, 1e-3,
        gdstk::make_tag(15, 0));
    roundEndPath->simple_path = true;
    roundEndPath->elements[0].end_type = gdstk::EndType::Round;
    extendedEndPath->elements[0].end_extensions =
        gdstk::Vec2{ 2.0, 3.0 };
    roundEndPath->segment(
        gdstk::Vec2{ 10.0, 0.0 },
        nullptr, nullptr, false);

    top->flexpath_array.append(flushEndPath);
    top->flexpath_array.append(extendedEndPath);
    top->flexpath_array.append(roundEndPath);
    library.cell_array.append(top);

    ASSERT_EQ(library.write_gds(filePath.c_str(), 0, nullptr), gdstk::ErrorCode::NoError);

    library.free_all();

    LayerRegistry registry;

    const Layer* m1 = registry.declare("M1");
    registry.mapGDS(m1, 15, 0);

    const auto shapes = GDSLayoutParser::load(filePath, registry);

    ASSERT_EQ(shapes.size(), 3);

    EXPECT_EQ(shapes[0].getLayer(), m1);
    EXPECT_EQ(shapes[1].getLayer(), m1);
    EXPECT_EQ(shapes[2].getLayer(), m1);

    const auto bounds0 = shapes[0].getPolygon().getBoundingBox();
    const auto bounds1 = shapes[1].getPolygon().getBoundingBox();
    const auto bounds2 = shapes[2].getPolygon().getBoundingBox();

    EXPECT_NEAR(bounds0.getMinX(), 0.0, EPSILON);
    EXPECT_NEAR(bounds0.getMaxX(), 10.0, EPSILON);
    EXPECT_NEAR(bounds0.getMinY(), -1.0, EPSILON);
    EXPECT_NEAR(bounds0.getMaxY(), 1.0, EPSILON);

    EXPECT_NEAR(bounds1.getMinX(), -2.0, EPSILON);
    EXPECT_NEAR(bounds1.getMaxX(), 13.0, EPSILON);
    EXPECT_NEAR(bounds1.getMinY(), -1.0, EPSILON);
    EXPECT_NEAR(bounds1.getMaxY(), 1.0, EPSILON);

    constexpr double PATH_TOLERANCE = 0.002;

    EXPECT_NEAR(bounds2.getMinX(), -1.0, PATH_TOLERANCE);
    EXPECT_NEAR(bounds2.getMaxX(), 11.0, PATH_TOLERANCE);
    EXPECT_NEAR(bounds2.getMinY(), -1.0, PATH_TOLERANCE);
    EXPECT_NEAR(bounds2.getMaxY(), 1.0, PATH_TOLERANCE);

    std::filesystem::remove(filePath);
}