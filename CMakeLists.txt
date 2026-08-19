cmake_minimum_required(VERSION 3.20)

project(DRChecker LANGUAGES CXX)

# --------------------------------------------------
# Dependencies
# --------------------------------------------------

include(FetchContent)

# nlohmann/json
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(json)

# --------------------------------------------------
# DRChecker library
# --------------------------------------------------

add_library(drchecker
    # Geometry
    src/geometry/Point.cpp
    src/geometry/Vector.cpp
    src/geometry/BoundingBox.cpp
    src/geometry/Segment.cpp
    src/geometry/Polygon.cpp

    # Domain
    src/domain/Shape.cpp
    src/domain/Violation.cpp
    src/domain/Layer.cpp

    # Rules
    src/rules/MinWidthRule.cpp
    src/rules/MinSpacingRule.cpp
    src/rules/MinEnclosureRule.cpp

    # Engine
    src/engine/DRCEngine.cpp

    # Parser
    src/io/JSONLayoutParser.cpp
    src/io/JSONRuleParser.cpp
    src/io/JSONReportWriter.cpp

    # Spatial
    src/spatial/QuadTree.cpp
    src/spatial/LayerSpatialIndex.cpp
)

target_include_directories(drchecker
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(drchecker PUBLIC cxx_std_20)

target_link_libraries(drchecker
    PUBLIC
        nlohmann_json::nlohmann_json
)

# --------------------------------------------------
# Tests
# --------------------------------------------------

option(DRCHECK_BUILD_TESTS "Build DRChecker tests" ON)

if(DRCHECK_BUILD_TESTS)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
    )

    # Recommended on Windows so GoogleTest uses
    # the same CRT as the project.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(drchecker_tests
        # Geometry Tests
        tests/geometry/PointTest.cpp
        tests/geometry/VectorTest.cpp
        tests/geometry/BoundingBoxTest.cpp
        tests/geometry/SegmentTest.cpp
        tests/geometry/PolygonTest.cpp

        # Domain tests
        tests/domain/ShapeTest.cpp
        tests/domain/ViolationTest.cpp
        tests/domain/LayerTest.cpp

        # Rules Tests
        tests/rules/MinWidthRuleTest.cpp
        tests/rules/MinSpacingRuleTest.cpp
        tests/rules/MinEnclosureRuleTest.cpp

        # Engine Tests
        tests/engine/DRCEngineTest.cpp

        # JSONParser Tests
        tests/io/JSONLayoutParserTest.cpp
        tests/io/JSONRuleParserTest.cpp
        tests/io/JSONReportWriterTest.cpp

        # EndToEnd Tests
        tests/integration/EndtoEndTest.cpp

        # Spatial
        tests/spatial/QuadTreeTest.cpp
        tests/spatial/LayerSpatialIndexTest.cpp
    )

    target_link_libraries(
        drchecker_tests
        PRIVATE
            drchecker
            GTest::gtest_main
    )

    target_compile_definitions(
        drchecker_tests
        PRIVATE
        DRCHECK_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
    )

    include(GoogleTest)

    gtest_discover_tests(drchecker_tests)
endif()

add_executable(
    drcheck
    src/main.cpp
)

target_link_libraries(
    drcheck
    PRIVATE
        drchecker
)

target_compile_definitions(
    drcheck
    PRIVATE
        DRCHECK_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

add_executable(
    drcheck_spacing_benchmark
    benchmarks/MinSpacingBenchmark.cpp
)

target_link_libraries(
    drcheck_spacing_benchmark
    PRIVATE
        drchecker
)

target_compile_definitions(
    drcheck_spacing_benchmark
    PRIVATE
        DRCHECK_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

add_executable(
    drcheck_enclosure_benchmark
    benchmarks/MinEnclosureBenchmark.cpp
)

target_link_libraries(
    drcheck_enclosure_benchmark
    PRIVATE
        drchecker
)

target_compile_definitions(
    drcheck_enclosure_benchmark
    PRIVATE
        DRCHECK_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)