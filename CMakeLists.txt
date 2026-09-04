cmake_minimum_required(VERSION 3.26)

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

# tcl
find_package(TCL QUIET)

if(NOT TCL_FOUND)
    message(FATAL_ERROR "Tcl development files not found")
endif()

# clipper2
set(CLIPPER2_UTILS OFF CACHE BOOL "" FORCE)
set(CLIPPER2_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CLIPPER2_TESTS OFF CACHE BOOL "" FORCE)
set(CLIPPER2_USINGZ "OFF" CACHE STRING "" FORCE)

FetchContent_Declare(
    clipper2
    GIT_REPOSITORY https://github.com/AngusJohnson/Clipper2.git
    GIT_TAG Clipper2_2.0.1
    SOURCE_SUBDIR CPP
)

FetchContent_MakeAvailable(clipper2)

# zlib (gdstk dependency)
set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ZLIB_INSTALL ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    ZLIB
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG v1.3.2
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(ZLIB)

if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()

# qhull (gdstk dependency)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    qhull
    GIT_REPOSITORY https://github.com/qhull/qhull.git
    GIT_TAG v8.0.2
)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

function(add_test)
    # Intentionally empty: swallows Qhull's internal add_test commands
endfunction()

FetchContent_MakeAvailable(qhull)
# Qhull 8.0.2 always defines its applications and test/example targets.
# DRCheck only needs qhullstatic_r through gdstk.
foreach(qhull_target
    qconvex
    qdelaunay
    qhalf
    qhull
    qvoronoi
    rbox
    testqset
    testqset_r
    user_eg
    user_eg2
    user_eg3
    qhullstatic
    qhullcpp
)
    if(TARGET ${qhull_target})
        set_target_properties(
            ${qhull_target}
            PROPERTIES
                EXCLUDE_FROM_ALL TRUE
        )
    endif()
endforeach()

macro(add_test)
    _add_test(${ARGV})
endmacro()

add_library(drcheck_qhull INTERFACE)

target_link_libraries(
    drcheck_qhull
    INTERFACE
        qhullstatic_r
)

target_include_directories(
    drcheck_qhull
    INTERFACE
        ${qhull_SOURCE_DIR}/src
)

add_library(
    QHULL::QHULL
    ALIAS
    drcheck_qhull
)

# gdstk
set(GDSTK_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    gdstk
    GIT_REPOSITORY https://github.com/heitzmann/gdstk.git
    GIT_TAG v1.0.1
)

FetchContent_MakeAvailable(gdstk)

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
    src/domain/LayerRegistry.cpp

    # Rules
    src/rules/MinWidthRule.cpp
    src/rules/MinSpacingRule.cpp
    src/rules/MinEnclosureRule.cpp
    src/rules/DensityRule.cpp
    src/rules/RuleFactory.cpp

    # Engine
    src/engine/DRCEngine.cpp
    src/engine/DRCRunner.cpp

    # I/O
    src/io/JSONLayoutParser.cpp
    src/io/JSONRuleParser.cpp
    src/io/JSONReportWriter.cpp
    src/io/SVGReportWriter.cpp
    src/io/TclRuleParser.cpp
    src/io/TclAutomationRunner.cpp
    src/io/GDSLayoutParser.cpp

    # Spatial
    src/spatial/QuadTree.cpp
    src/spatial/LayerSpatialIndex.cpp

    # Layout
    src/layout/LayoutNormalizer.cpp
)

target_include_directories(drchecker
    # Public: Targets using my target can access it
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    # Private: Only targets under drchecker can access it
    PRIVATE
        ${TCL_INCLUDE_PATH}
)

target_link_libraries(drchecker
    PRIVATE
        nlohmann_json::nlohmann_json
        ${TCL_LIBRARY}
        Clipper2
        gdstk::gdstk
)

target_compile_features(drchecker PUBLIC cxx_std_20)

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
        tests/domain/LayerRegistryTest.cpp

        # Rules Tests
        tests/rules/MinWidthRuleTest.cpp
        tests/rules/MinSpacingRuleTest.cpp
        tests/rules/MinEnclosureRuleTest.cpp
        tests/rules/DensityRuleTest.cpp
        tests/rules/RuleFactoryTest.cpp

        # Engine Tests
        tests/engine/DRCEngineTest.cpp
        tests/engine/DRCRunnerTest.cpp

        # I/O Tests
        tests/io/JSONLayoutParserTest.cpp
        tests/io/JSONRuleParserTest.cpp
        tests/io/JSONReportWriterTest.cpp
        tests/io/SVGReportWriterTest.cpp
        tests/io/TclRuleParserTest.cpp
        tests/io/TclAutomationRunnerTest.cpp
        tests/io/GDSTKIntegrationTest.cpp
        tests/io/GDSLayoutParserTest.cpp

        # EndToEnd Tests
        tests/integration/EndtoEndTest.cpp

        # Spatial
        tests/spatial/QuadTreeTest.cpp
        tests/spatial/LayerSpatialIndexTest.cpp

        # Layout
        tests/layout/LayoutNormalizerTest.cpp
    )

    target_link_libraries(
        drchecker_tests
        PRIVATE
            drchecker
            GTest::gtest_main
            nlohmann_json::nlohmann_json
            gdstk::gdstk
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

