#include <gtest/gtest.h>
#include <gdstk/gdstk.hpp>
#include <filesystem>
#include <string>
#include <numbers>

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
    const std::string filePath = "basic_layout.gds";

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

    std::filesystem::remove(filePath);
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

TEST(GDSLayoutParserTest, ImportsTranslatedSREF)
{
    const std::string filePath = "gds_sref_translation_test.gds";

    gdstk::Library library{};
    library.init("TEST_LIBRARY", 1e-6, 1e-9);

    auto* top = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    top->init("TOP");

    auto* child = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    child->init("CHILD");

    auto* rectangle = static_cast<gdstk::Polygon*>(gdstk::allocate_clear(sizeof(gdstk::Polygon)));

    *rectangle = gdstk::rectangle(
        gdstk::Vec2{ 0.0, 0.0 },
        gdstk::Vec2{ 2.0, 1.0 },
        gdstk::make_tag(15, 0)
    );

    child->polygon_array.append(rectangle);

    auto* firstReference = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));
    firstReference->init(child);

    firstReference->origin = gdstk::Vec2{ 10.0, 20.0 };

    top->reference_array.append(firstReference);

    auto* secondReference = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));
    secondReference->init(child);

    // Rotate it by 90° around the reference origin
    secondReference->rotation = M_PI / 2.0;

    top->reference_array.append(secondReference);

    auto* thirdReference = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));
    thirdReference->init(child);

	// Reflect it across the X-axis around the reference origin
    thirdReference->x_reflection = true;

    top->reference_array.append(thirdReference);

    auto* fourthReference = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));
    fourthReference->init(child);

	// Scale it by 2x around the reference origin
    fourthReference->magnification = 2.0;

    top->reference_array.append(fourthReference);

    library.cell_array.append(top);
    library.cell_array.append(child);

    ASSERT_EQ(library.write_gds(filePath.c_str(), 0, nullptr), gdstk::ErrorCode::NoError);

    library.free_all();

    LayerRegistry registry;
    const Layer* m1 = registry.declare("M1");
    registry.mapGDS(m1, 15, 0);

    const auto shapes = GDSLayoutParser::load(filePath, registry);

    ASSERT_EQ(shapes.size(), 4);
    EXPECT_EQ(shapes[0].getLayer(), m1);
    EXPECT_EQ(shapes[1].getLayer(), m1);
    EXPECT_EQ(shapes[2].getLayer(), m1);
    EXPECT_EQ(shapes[3].getLayer(), m1);

    const auto bounds0 = shapes[0].getPolygon().getBoundingBox();
    const auto bounds1 = shapes[1].getPolygon().getBoundingBox();
    const auto bounds2 = shapes[2].getPolygon().getBoundingBox();
    const auto bounds3 = shapes[3].getPolygon().getBoundingBox();

    EXPECT_NEAR(bounds0.getMinX(), 10.0, EPSILON);
    EXPECT_NEAR(bounds0.getMinY(), 20.0, EPSILON);
    EXPECT_NEAR(bounds0.getMaxX(), 12.0, EPSILON);
    EXPECT_NEAR(bounds0.getMaxY(), 21.0, EPSILON);

    EXPECT_NEAR(bounds1.getMinX(), -1.0, EPSILON);
    EXPECT_NEAR(bounds1.getMinY(), 0.0, EPSILON);
    EXPECT_NEAR(bounds1.getMaxX(), 0.0, EPSILON);
    EXPECT_NEAR(bounds1.getMaxY(), 2.0, EPSILON);

	EXPECT_NEAR(bounds2.getMinX(), 0.0, EPSILON);
    EXPECT_NEAR(bounds2.getMinY(), -1.0, EPSILON);
    EXPECT_NEAR(bounds2.getMaxX(), 2.0, EPSILON);
    EXPECT_NEAR(bounds2.getMaxY(), 0.0, EPSILON);

    EXPECT_NEAR(bounds3.getMinX(), 0.0, EPSILON);
    EXPECT_NEAR(bounds3.getMinY(), 0.0, EPSILON);
    EXPECT_NEAR(bounds3.getMaxX(), 4.0, EPSILON);
    EXPECT_NEAR(bounds3.getMaxY(), 2.0, EPSILON);

    std::filesystem::remove(filePath);
}



TEST(GDSLayoutParserTest, ImportsTranslatedNestedSREF)
{
    const std::string filePath = "gds_nested_sref_translation_test.gds";
    // Initialize library
    gdstk::Library library{};
    library.init("TEST_LIBRARY", 1e-6, 1e-9);
    // Initialize empty cells
    auto* top = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    top->init("TOP");

    auto* mid = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    mid->init("MID");

    auto* child = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    child->init("CHILD");
	// Add rectangle to child cell
    auto* rectangle = static_cast<gdstk::Polygon*>(gdstk::allocate_clear(sizeof(gdstk::Polygon)));
    *rectangle = gdstk::rectangle(
        gdstk::Vec2{ 0.0, 0.0 },
        gdstk::Vec2{ 2.0, 1.0 },
        gdstk::make_tag(15, 0)
    );
    child->polygon_array.append(rectangle);

	// create a reference to the child cell and add it to the mid cell
    auto* translatedChildReference = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));

    translatedChildReference->init(child);
    translatedChildReference->origin = gdstk::Vec2{ 5.0, 10.0 };
    mid->reference_array.append(translatedChildReference);

    auto* rotatedChildReference = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));

    rotatedChildReference->init(child);
    rotatedChildReference->rotation = M_PI / 2.0;
    mid->reference_array.append(rotatedChildReference);

	// create a reference to the mid cell and add it to the top cell
    auto* midReference = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));

    midReference->init(mid);
    midReference->origin = gdstk::Vec2{ 20.0, 30.0 };

    top->reference_array.append(midReference);

	// add the cells to the library
    library.cell_array.append(top);
	library.cell_array.append(mid);
    library.cell_array.append(child);

    ASSERT_EQ(library.write_gds(filePath.c_str(), 0, nullptr), gdstk::ErrorCode::NoError);

    library.free_all();

    LayerRegistry registry;
    const Layer* m1 = registry.declare("M1");
    registry.mapGDS(m1, 15, 0);

    const auto shapes = GDSLayoutParser::load(filePath, registry);

    ASSERT_EQ(shapes.size(), 2);
    EXPECT_EQ(shapes[0].getLayer(), m1);
    EXPECT_EQ(shapes[1].getLayer(), m1);

    const auto bounds0 = shapes[0].getPolygon().getBoundingBox();
    const auto bounds1 = shapes[1].getPolygon().getBoundingBox();

    EXPECT_NEAR(bounds0.getMinX(), 25.0, EPSILON);
    EXPECT_NEAR(bounds0.getMinY(), 40.0, EPSILON);
    EXPECT_NEAR(bounds0.getMaxX(), 27.0, EPSILON);
    EXPECT_NEAR(bounds0.getMaxY(), 41.0, EPSILON);
    
    EXPECT_NEAR(bounds1.getMinX(), 19.0, EPSILON);
    EXPECT_NEAR(bounds1.getMinY(), 30.0, EPSILON);
    EXPECT_NEAR(bounds1.getMaxX(), 20.0, EPSILON);
    EXPECT_NEAR(bounds1.getMaxY(), 32.0, EPSILON);

    std::filesystem::remove(filePath);
}

TEST(GDSLayoutParserTest, RejectsRecursiveHierarchy)
{
    //  TOP
    //   └── A
    //       └── B
    //           └── A    ← cycle
    const std::string filePath = "gds_recursive_hierarchy_test.gds";

    gdstk::Library library{};
    library.init("TEST_LIBRARY", 1e-6, 1e-9);

    auto* top = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    top->init("TOP");

    auto* cellA = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    cellA->init("A");

    auto* cellB = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    cellB->init("B");

    auto* topToA = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));

    topToA->init(cellA);
    top->reference_array.append(topToA);

    auto* aToB = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));

    aToB->init(cellB);
    cellA->reference_array.append(aToB);

    auto* bToA = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));

    bToA->init(cellA);
    cellB->reference_array.append(bToA);

    library.cell_array.append(top);
    library.cell_array.append(cellA);
    library.cell_array.append(cellB);

    ASSERT_EQ(library.write_gds(filePath.c_str(), 0, nullptr), gdstk::ErrorCode::NoError);

    library.free_all();

    LayerRegistry registry;

    EXPECT_THROW(GDSLayoutParser::load(filePath, registry), std::invalid_argument);

    std::filesystem::remove(filePath);
}

TEST(GDSLayoutParserTest, ImportsTranslatedAREF)
{
    const std::string filePath = "gds_aref_translation_test.gds";

    gdstk::Library library{};
    library.init("TEST_LIBRARY", 1e-6, 1e-9);

    auto* top = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    top->init("TOP");

    auto* child = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    child->init("CHILD");

    auto* rectangle = static_cast<gdstk::Polygon*>(gdstk::allocate_clear(sizeof(gdstk::Polygon)));

    *rectangle = gdstk::rectangle(
        gdstk::Vec2{ 0.0, 0.0 },
        gdstk::Vec2{ 2.0, 1.0 },
        gdstk::make_tag(15, 0)
    );

    child->polygon_array.append(rectangle);

	// create an array reference to the child cell and add it to the top cell
    auto* reference = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));

    reference->init(child);
    reference->origin = gdstk::Vec2{ 10.0, 20.0 };
    reference->repetition.type = gdstk::RepetitionType::Rectangular;
    reference->repetition.columns = 3;
    reference->repetition.rows = 2;
    reference->repetition.spacing = gdstk::Vec2{ 5.0, 4.0 };

    top->reference_array.append(reference);

    library.cell_array.append(top);
    library.cell_array.append(child);

    ASSERT_EQ(library.write_gds(filePath.c_str(), 0, nullptr), gdstk::ErrorCode::NoError);

    library.free_all();

    LayerRegistry registry;
    const Layer* m1 = registry.declare("M1");
    registry.mapGDS(m1, 15, 0);

    const auto shapes = GDSLayoutParser::load(filePath, registry);

    ASSERT_EQ(shapes.size(), 6);

    for (const auto& shape : shapes)
    {
        EXPECT_EQ(shape.getLayer(), m1);
    }

    std::vector<std::pair<double, double>> lowerLeftCorners;

    for (const auto& shape : shapes)
    {
        const auto bounds = shape.getPolygon().getBoundingBox();

        lowerLeftCorners.emplace_back(bounds.getMinX(), bounds.getMinY());

        EXPECT_NEAR(bounds.getMaxX() - bounds.getMinX(), 2.0, EPSILON);
        EXPECT_NEAR(bounds.getMaxY() - bounds.getMinY(), 1.0, EPSILON);
    }

    std::sort(lowerLeftCorners.begin(), lowerLeftCorners.end());

    const std::vector<std::pair<double, double>>
        expectedCorners{
            {10.0, 20.0},
            {10.0, 24.0},
            {15.0, 20.0},
            {15.0, 24.0},
            {20.0, 20.0},
            {20.0, 24.0}
    };

    EXPECT_EQ(lowerLeftCorners, expectedCorners);

    std::filesystem::remove(filePath);
}

TEST(GDSLayoutParserTest, ImportsTransformedAREF)
{
    const std::string filePath = "gds_aref_translation_test.gds";

    gdstk::Library library{};
    library.init("TEST_LIBRARY", 1e-6, 1e-9);

    auto* top = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    top->init("TOP");

    auto* child = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    child->init("CHILD");

    auto* rectangle = static_cast<gdstk::Polygon*>(gdstk::allocate_clear(sizeof(gdstk::Polygon)));

    *rectangle = gdstk::rectangle(
        gdstk::Vec2{ 0.0, 0.0 },
        gdstk::Vec2{ 2.0, 1.0 },
        gdstk::make_tag(15, 0)
    );

    child->polygon_array.append(rectangle);

    // create an array reference to the child cell and add it to the top cell
    auto* reference = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));

    reference->init(child);
    reference->origin = gdstk::Vec2{ 10.0, 20.0 };
    reference->rotation = M_PI / 2.0;
    reference->repetition.type = gdstk::RepetitionType::Rectangular;
    reference->repetition.columns = 3;
    reference->repetition.rows = 2;
    reference->repetition.spacing = gdstk::Vec2{ 5.0, 4.0 };

    top->reference_array.append(reference);

    library.cell_array.append(top);
    library.cell_array.append(child);

    ASSERT_EQ(library.write_gds(filePath.c_str(), 0, nullptr), gdstk::ErrorCode::NoError);

    library.free_all();

    LayerRegistry registry;
    const Layer* m1 = registry.declare("M1");
    registry.mapGDS(m1, 15, 0);

    const auto shapes = GDSLayoutParser::load(filePath, registry);

    ASSERT_EQ(shapes.size(), 6);

    for (const auto& shape : shapes)
    {
        EXPECT_EQ(shape.getLayer(), m1);
    }

    std::vector<std::pair<double, double>> lowerLeftCorners;

    for (const auto& shape : shapes)
    {
        const auto bounds = shape.getPolygon().getBoundingBox();

        lowerLeftCorners.emplace_back(bounds.getMinX(), bounds.getMinY());

        EXPECT_NEAR(bounds.getMaxX() - bounds.getMinX(), 1.0, EPSILON);
        EXPECT_NEAR(bounds.getMaxY() - bounds.getMinY(), 2.0, EPSILON);
    }

    std::sort(lowerLeftCorners.begin(), lowerLeftCorners.end());

    const std::vector<std::pair<double, double>>
        expectedCorners{
            {9.0, 20.0},
            {9.0, 24.0},
            {14.0, 20.0},
            {14.0, 24.0},
            {19.0, 20.0},
            {19.0, 24.0}
    };

    EXPECT_EQ(lowerLeftCorners, expectedCorners);

    std::filesystem::remove(filePath);
}

TEST(GDSLayoutParserTest, SelectsRequestedTopLevelCell)
{
    const std::string filePath = "gds_select_top_cell_test.gds";

    gdstk::Library library{};
    library.init("TEST_LIBRARY", 1e-6, 1e-9);

    // ---------------- TOP_A ----------------

    auto* topA = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    topA->init("TOP_A");

    auto* rectangleA = static_cast<gdstk::Polygon*>(gdstk::allocate_clear(sizeof(gdstk::Polygon)));

    *rectangleA = gdstk::rectangle(
        gdstk::Vec2{ 0.0, 0.0 },
        gdstk::Vec2{ 2.0, 2.0 },
        gdstk::make_tag(15, 0)
    );

    topA->polygon_array.append(rectangleA);

    // ---------------- TOP_B ----------------

    auto* topB = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));
    topB->init("TOP_B");

    auto* rectangleB = static_cast<gdstk::Polygon*>(gdstk::allocate_clear(sizeof(gdstk::Polygon)));

    *rectangleB = gdstk::rectangle(
        gdstk::Vec2{ 10.0, 20.0 },
        gdstk::Vec2{ 14.0, 25.0 },
        gdstk::make_tag(15, 0)
    );

    topB->polygon_array.append(rectangleB);

    library.cell_array.append(topA);
    library.cell_array.append(topB);

    ASSERT_EQ(library.write_gds(filePath.c_str(), 0, nullptr), gdstk::ErrorCode::NoError);

    library.free_all();

    LayerRegistry registry;
    const Layer* m1 = registry.declare("M1");
    registry.mapGDS(m1, 15, 0);

    const auto shapes = GDSLayoutParser::load(filePath, registry, std::string("TOP_B"));

    ASSERT_EQ(shapes.size(), 1);
    EXPECT_EQ(shapes[0].getLayer(), m1);

    const auto bounds = shapes[0].getPolygon().getBoundingBox();

    EXPECT_NEAR(bounds.getMinX(), 10.0, EPSILON);
    EXPECT_NEAR(bounds.getMinY(), 20.0, EPSILON);
    EXPECT_NEAR(bounds.getMaxX(), 14.0, EPSILON);
    EXPECT_NEAR(bounds.getMaxY(), 25.0, EPSILON);

    std::filesystem::remove(filePath);
}

TEST(GDSLayoutParserTest, RejectsUnknownRequestedTopLevelCell)
{
    const std::string filePath = "gds_unknown_top_cell_test.gds";

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

    EXPECT_THROW(GDSLayoutParser::load(filePath, registry, std::string("DOES_NOT_EXIST")), std::invalid_argument);

    std::filesystem::remove(filePath);
}