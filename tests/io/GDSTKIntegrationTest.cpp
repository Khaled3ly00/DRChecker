#include <gtest/gtest.h>
#include <gdstk/gdstk.hpp>
#include <filesystem>
#include <string>

TEST(GDSTKIntegrationTest, LibraryIsLinked)
{
    double unit = 0.0;
    double precision = 0.0;

    const gdstk::ErrorCode error = gdstk::gds_units("file_that_does_not_exist.gds", unit, precision);

    EXPECT_EQ(error, gdstk::ErrorCode::InputFileOpenError);
}

TEST(GDSTKIntegrationTest, ReadsLibraryMetadata)
{
    const std::string filePath = "gdstk_metadata_test.gds";

    gdstk::Library library{};
    library.init("TEST_LIBRARY", 1e-6, 1e-9);

    auto* cell = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));

    cell->init("TOP");
    library.cell_array.append(cell);

    ASSERT_EQ(library.write_gds(filePath.c_str(), 0, nullptr), gdstk::ErrorCode::NoError);

    library.free_all();

    gdstk::ErrorCode errorCode = gdstk::ErrorCode::NoError;

    gdstk::Library loadedLibrary = gdstk::read_gds(filePath.c_str(), 0, 0, nullptr, &errorCode);

    ASSERT_EQ(errorCode, gdstk::ErrorCode::NoError);
    EXPECT_DOUBLE_EQ(loadedLibrary.unit, 1e-6);
    EXPECT_DOUBLE_EQ(loadedLibrary.precision, 1e-9);
    ASSERT_EQ(loadedLibrary.cell_array.count, 1);
    EXPECT_STREQ(loadedLibrary.cell_array[0]->name, "TOP");

    loadedLibrary.free_all();

    std::filesystem::remove(filePath);
}

/**
 * TEST: FindsTopLevelCell
 *
 * Purpose:
 *   Validate the library's ability to identify top-level cells (cells not
 *   referenced by any other cell) after writing and reading back a GDS file.
 *
 * Behavior:
 *   - Create a library with two cells: "TOP" and "CHILD".
 *   - Make "TOP" reference "CHILD" (so "TOP" remains top-level, "CHILD" is not).
 *   - Write to a temporary GDS file, free the in-memory structures, and read back.
 *   - Call `top_level` on the loaded library and assert:
 *       * exactly one top-level cell is found ("TOP"),
 *       * there are no top-level raw cells.
 *   - Clean up by freeing the loaded library and removing the temporary file.
 */
TEST(GDSTKIntegrationTest, FindsTopLevelCell)
{
    const std::string filePath = "gdstk_top_level_test.gds";

    gdstk::Library library{};
    library.init("TEST_LIBRARY", 1e-6, 1e-9);

    auto* top = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));

    top->init("TOP");

    auto* child = static_cast<gdstk::Cell*>(gdstk::allocate_clear(sizeof(gdstk::Cell)));

    child->init("CHILD");

    auto* reference = static_cast<gdstk::Reference*>(gdstk::allocate_clear(sizeof(gdstk::Reference)));

    reference->init(child);

    top->reference_array.append(reference);

    library.cell_array.append(top);
    library.cell_array.append(child);

    ASSERT_EQ(library.write_gds(filePath.c_str(), 0, nullptr), gdstk::ErrorCode::NoError);

    library.free_all();

    gdstk::ErrorCode errorCode = gdstk::ErrorCode::NoError;
    gdstk::Library loadedLibrary = gdstk::read_gds(filePath.c_str(), 0, 0, nullptr, &errorCode);

    ASSERT_EQ(errorCode, gdstk::ErrorCode::NoError);

    gdstk::Array<gdstk::Cell*> topCells{};
    gdstk::Array<gdstk::RawCell*> topRawCells{};

    loadedLibrary.top_level(topCells, topRawCells);

    ASSERT_EQ(topCells.count, 1);
    EXPECT_STREQ(topCells[0]->name, "TOP");

    EXPECT_EQ(topRawCells.count, 0);
    // clear the temp arrays
    topCells.clear();
    topRawCells.clear();
	// clear the loaded library
    loadedLibrary.free_all();

    std::filesystem::remove(filePath);
}