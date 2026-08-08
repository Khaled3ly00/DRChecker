cmake_minimum_required(VERSION 3.10)

project(DRChecker)

include(FetchContent)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
        "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"
    )
endif()

# --------------------------------------------------
# DRChecker library
# --------------------------------------------------

add_library(drchecker
    src/geometry/Point.cpp
    src/geometry/Vector.cpp
    src/geometry/BoundingBox.cpp
    src/geometry/Segment.cpp
)

target_include_directories(drchecker
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(drchecker PUBLIC cxx_std_20)

# --------------------------------------------------
# GoogleTest
# --------------------------------------------------

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

# Recommended on Windows so GoogleTest uses the same CRT as the project.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# --------------------------------------------------
# Tests
# --------------------------------------------------

enable_testing()

add_executable(drchecker_tests
    tests/geometry/PointTest.cpp
    tests/geometry/VectorTest.cpp
    tests/geometry/BoundingBoxTest.cpp
    tests/geometry/SegmentTest.cpp
)

target_link_libraries(drchecker_tests
    PRIVATE
        drchecker
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(drchecker_tests)