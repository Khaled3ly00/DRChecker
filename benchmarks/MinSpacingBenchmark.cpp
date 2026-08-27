#include <chrono>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
#include <stdexcept>

#include "drcheck/domain/Shape.h"
#include "drcheck/geometry/Constants.h"
#include "drcheck/geometry/Point.h"
#include "drcheck/geometry/Polygon.h"
#include "drcheck/spatial/QuadTree.h"
#include "drcheck/geometry/BoundingBox.h"

struct BenchmarkResult
{
    std::size_t pairChecks = 0;
    std::size_t violationCount = 0;
    double milliseconds = 0.0;
};

// Return a vector of n shapes with specific width, height, spacing
std::vector<drcheck::domain::Shape> generateGrid(std::size_t count, double width, double height, double gap)
{
    using drcheck::domain::Layer;
    using drcheck::domain::Purpose;
    using drcheck::domain::Shape;
    using drcheck::geometry::Point;
    using drcheck::geometry::Polygon;

    std::vector<Shape> shapes;
    shapes.reserve(count);
    
    // Arrange shapes approximately in a square grid.
    // std::sqrt accepts double only
    const std::size_t columns = static_cast<std::size_t>(std::ceil(std::sqrt(static_cast<double>(count))));

    // Loop through each column and row to create shapes
    for (std::size_t i = 0; i < count; ++i)
    {
        const std::size_t row = i / columns;
        const std::size_t column = i % columns;

        const double x = column * (width + gap);
        const double y = row * (height + gap);

        Polygon polygon({
            Point(x, y),
            Point(x + width, y),
            Point(x + width, y + height),
            Point(x, y + height)
            });

        shapes.emplace_back(i, Layer::M1, Purpose::Drawing, std::move(polygon));
    }

    return shapes;
}

BenchmarkResult runBruteForce(const std::vector<drcheck::domain::Shape>& shapes, double minimumSpacing)
{
    using namespace drcheck;

    BenchmarkResult result;
    // start clock
    const auto start = std::chrono::steady_clock::now();

    for (std::size_t i = 0; i < shapes.size(); ++i)
    {
        if (shapes[i].getLayer() != domain::Layer::M1)
        {
            continue;
        }

        for (std::size_t j = i + 1; j < shapes.size(); ++j)
        {
            if (shapes[j].getLayer() != domain::Layer::M1)
            {
                continue;
            }
  
            ++result.pairChecks;

            const double spacing = shapes[i].getPolygon().distanceTo(shapes[j].getPolygon()).distance;

            if (spacing + geometry::EPSILON < minimumSpacing)
            {
                ++result.violationCount;
            }
        }
    }
    // end clock
    const auto end = std::chrono::steady_clock::now();

    result.milliseconds = std::chrono::duration<double, std::milli>(end - start).count();

    return result;
}

BenchmarkResult runQuadtree(const std::vector<drcheck::domain::Shape>& shapes, double minimumSpacing, std::size_t capacity, std::size_t maxDepth)
{
    using namespace drcheck;

    BenchmarkResult result;

    const auto start = std::chrono::steady_clock::now();

    const domain::Shape* firstShape = nullptr;

    for (const domain::Shape& shape : shapes)
    {
        if (shape.getLayer() == domain::Layer::M1)
        {
            firstShape = &shape;
            break;
        }
    }

    if (!firstShape) {
        return result;
    }

    geometry::BoundingBox boundary = firstShape->getPolygon().getBoundingBox();

    for (const domain::Shape& shape : shapes)
    {
        if (shape.getLayer() != domain::Layer::M1)
        {
            continue;
        }

        boundary = boundary.mergedWith(shape.getPolygon().getBoundingBox());
    }

    spatial::QuadTree tree(boundary, capacity, maxDepth);

    for (const domain::Shape& shape : shapes)
    {
        if (shape.getLayer() == domain::Layer::M1)
        {
            tree.insert(shape);
        }
    }

    for (const domain::Shape& shape : shapes)
    {
        if (shape.getLayer() != domain::Layer::M1)
        {
            continue;
        }

        const geometry::BoundingBox searchRegion = shape.getPolygon().getBoundingBox().expanded(minimumSpacing);

        const auto candidates = tree.query(searchRegion);

        for (const domain::Shape* candidate :candidates)
        {
            if (candidate == &shape) {
                continue;
            }

            if (candidate->getId() <= shape.getId())
            {
                continue;
            }

            ++result.pairChecks;

            const double spacing = shape.getPolygon().distanceTo(candidate->getPolygon()).distance;

            if (spacing + geometry::EPSILON < minimumSpacing)
            {
                ++result.violationCount;
            }
        }
    }

    const auto end = std::chrono::steady_clock::now();

    result.milliseconds = std::chrono::duration<double, std::milli>(end - start).count();

    return result;
}

// Calculates average run time for n repitions of QuadTree
BenchmarkResult runQuadtreeAverage(const std::vector<drcheck::domain::Shape>& shapes, double minimumSpacing, std::size_t capacity, std::size_t maxDepth, std::size_t repetitions)
{
    if (repetitions == 0)
    {
        throw std::invalid_argument("Benchmark repetitions must be greater than zero");
    }

    BenchmarkResult averageResult;

    // Warm-up run.
    // This result is intentionally not included in the average.
    runQuadtree(shapes, minimumSpacing, capacity, maxDepth);

    double totalMilliseconds = 0.0;

    for (std::size_t i = 0; i < repetitions; ++i)
    {
        const BenchmarkResult result = runQuadtree(shapes, minimumSpacing, capacity, maxDepth);

        totalMilliseconds += result.milliseconds;

        // Pair checks and violation count should be identical
        // for every repetition.
        averageResult.pairChecks = result.pairChecks;
        averageResult.violationCount = result.violationCount;
    }

    averageResult.milliseconds = totalMilliseconds / static_cast<double>(repetitions);

    return averageResult;
}

int main()
{
    constexpr double width = 10.0;
    constexpr double height = 10.0;

    constexpr double denseGap = 2.0;
    constexpr double sparseGap = 10.0;

    constexpr double minimumSpacing = 3.0;

    constexpr std::size_t shapeCount = 2000;
    constexpr std::size_t repetitions = 5;

    const std::vector<std::size_t> capacities{2, 4, 8, 16};

    const std::vector<std::size_t> maxDepths{ 4, 6, 8, 10};

    // Generate both layouts once.
    const auto denseShapes = generateGrid(shapeCount, width, height, denseGap);

    const auto sparseShapes = generateGrid(shapeCount, width, height, sparseGap);

    // Our already-tested configuration is used as
    // the correctness reference.
    constexpr std::size_t referenceCapacity = 4;
    constexpr std::size_t referenceMaxDepth = 8;

    const BenchmarkResult denseReference = runQuadtree(denseShapes, minimumSpacing, referenceCapacity, referenceMaxDepth);

    const BenchmarkResult sparseReference =runQuadtree(sparseShapes, minimumSpacing, referenceCapacity, referenceMaxDepth);

    const std::filesystem::path outputPath = std::filesystem::path(DRCHECK_SOURCE_DIR)/"benchmarks"/"results"/"quadtree_parameter_tuning.txt";
    std::filesystem::create_directories(outputPath.parent_path());
    std::ofstream outputFile(outputPath);

    if (!outputFile)
    {
        throw std::runtime_error("Unable to open benchmark output file: " + outputPath.string());
    }

    auto printLine =
        [&outputFile](const std::string& text)
        {
            std::cout << text << '\n';
            outputFile << text << '\n';
        };

    printLine("DRCheck QuadTree Parameter Tuning");
    printLine("=================================");
    printLine("Shape Count: " + std::to_string(shapeCount));
    printLine("Minimum Spacing: " + std::to_string(minimumSpacing));
    printLine("Repetitions Per Configuration: " + std::to_string(repetitions));
    printLine("");
    printLine("Reference Results");
    printLine("-----------------");
    printLine("Dense Pair Checks: " + std::to_string(denseReference.pairChecks));
    printLine("Dense Violations: " + std::to_string(denseReference.violationCount));
    printLine("Sparse Pair Checks: " + std::to_string(sparseReference.pairChecks));
    printLine("Sparse Violations: " + std::to_string(sparseReference.violationCount));
    printLine("");
    printLine("=================================");
    printLine("");

    for (const std::size_t capacity : capacities)
    {
        for (const std::size_t maxDepth : maxDepths)
        {
            const BenchmarkResult dense = runQuadtreeAverage(denseShapes, minimumSpacing, capacity, maxDepth, repetitions);
            const BenchmarkResult sparse = runQuadtreeAverage(sparseShapes, minimumSpacing, capacity, maxDepth, repetitions);

            printLine("Capacity: " + std::to_string(capacity));
            printLine("Max Depth: " + std::to_string(maxDepth));
            printLine("");
            printLine("Dense");
            printLine("  Pair checks: " + std::to_string(dense.pairChecks));
            printLine("  Violations: " + std::to_string(dense.violationCount));
            printLine("  Average time: " + std::to_string(dense.milliseconds) + " ms");
            printLine("");
            printLine("Sparse");
            printLine("  Pair checks: " + std::to_string(sparse.pairChecks));
            printLine("  Violations: " + std::to_string(sparse.violationCount));
            printLine("  Average time: " + std::to_string(sparse.milliseconds) + " ms");

            // Validate dense correctness.
            if (
                dense.pairChecks != denseReference.pairChecks || dense.violationCount != denseReference.violationCount)
            {
                printLine("*** DENSE RESULT MISMATCH ***");
            }

            // Validate sparse correctness.
            if (sparse.pairChecks != sparseReference.pairChecks || sparse.violationCount != sparseReference.violationCount)
            {
                printLine("*** SPARSE RESULT MISMATCH ***");
            }

            printLine("");
            printLine("-----------------------------");
            printLine("");
        }
    }

    printLine("Parameter tuning completed successfully.");

    std::cout
        << "\nResults written to: "
        << outputPath
        << '\n';

    return 0;
}