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
#include "drcheck/spatial/LayerSpatialIndex.h"

using drcheck::domain::Layer;
using drcheck::domain::Purpose;
using drcheck::domain::Shape;

using drcheck::geometry::BoundingBox;
using drcheck::geometry::EPSILON;
using drcheck::geometry::Point;
using drcheck::geometry::Polygon;

using drcheck::spatial::LayerSpatialIndex;

struct EnclosureBenchmarkResult
{
    std::size_t containmentChecks;
    std::size_t violationCount;
    double milliseconds;
};

std::vector<Shape> generateEnclosureGrid(std::size_t count, double viaSize, double enclosure, double gap)
{
    std::vector<Shape> shapes;
    shapes.reserve(count * 2);

    const std::size_t columns = static_cast<std::size_t>(std::ceil(std::sqrt(static_cast<double>(count))));

    const double metalSize = viaSize + (enclosure * 2.0);
    const double pitch = metalSize + gap;

    for (std::size_t i = 0; i < count; ++i)
    {
        const std::size_t row = i / columns;
        const std::size_t column = i % columns;

        const double x = static_cast<double>(column) * pitch;
        const double y = static_cast<double>(row) * pitch;

        Polygon metalPolygon({
            Point(x, y),
            Point(x + metalSize, y),
            Point(x + metalSize, y + metalSize),
            Point(x, y + metalSize)
            });

        Polygon viaPolygon({
            Point(x + enclosure, y + enclosure),
            Point(x + enclosure + viaSize, y + enclosure),
            Point(x + enclosure + viaSize, y + enclosure + viaSize),
            Point(x + enclosure, y + enclosure + viaSize)
            });

        shapes.emplace_back(2 * i, Layer::M1, Purpose::Drawing, std::move(metalPolygon));
        shapes.emplace_back(2 * i + 1, Layer::VIA1, Purpose::Drawing, std::move(viaPolygon));
    }

    return shapes;
}

EnclosureBenchmarkResult runBruteForce(const std::vector<Shape>& shapes, double minimumEnclosure)
{
    std::size_t containmentChecks = 0;
    std::size_t violationCount = 0;

    const auto start = std::chrono::steady_clock::now();

    for (const Shape& innerShape : shapes)
    {
        if (innerShape.getLayer() != Layer::VIA1)
        {
            continue;
        }

        bool foundContainingOuter = false;

        for (const Shape& outerShape : shapes)
        {
            if (outerShape.getLayer() != Layer::M1)
            {
                continue;
            }

            ++containmentChecks;

            if (!outerShape.getPolygon().contains(innerShape.getPolygon()))
            {
                continue;
            }

            foundContainingOuter = true;

            const auto enclosureResult = innerShape.getPolygon().distanceTo(outerShape.getPolygon(), false);

            if (enclosureResult.distance + EPSILON < minimumEnclosure)
            {
                ++violationCount;
            }

            // Current project assumption:
            // at most one containing outer.
            break;
        }

        if (!foundContainingOuter)
        {
            ++violationCount;
        }
    }

    const auto end = std::chrono::steady_clock::now();

    const double milliseconds = std::chrono::duration<double, std::milli> (end - start).count();

    return {containmentChecks, violationCount, milliseconds};
}

EnclosureBenchmarkResult runSpatial(const std::vector<Shape>& shapes, const LayerSpatialIndex& spatialIndex, double minimumEnclosure)
{
    std::size_t containmentChecks = 0;
    std::size_t violationCount = 0;

    const auto start = std::chrono::steady_clock::now();

    for (const Shape& innerShape : shapes)
    {
        if (innerShape.getLayer() != Layer::VIA1)
        {
            continue;
        }

        bool foundContainingOuter = false;

        const BoundingBox innerBox = innerShape.getPolygon().getBoundingBox();

        const auto candidates = spatialIndex.query(Layer::M1,innerBox);

        for (const Shape* outerShape : candidates)
        {
            ++containmentChecks;

            if (!outerShape->getPolygon().contains(innerShape.getPolygon()))
            {
                continue;
            }

            foundContainingOuter = true;

            const auto enclosureResult = innerShape.getPolygon().distanceTo(outerShape->getPolygon(), false);

            if (enclosureResult.distance + EPSILON < minimumEnclosure)
            {
                ++violationCount;
            }

            break;
        }

        if (!foundContainingOuter)
        {
            ++violationCount;
        }
    }

    const auto end = std::chrono::steady_clock::now();

    const double milliseconds = std::chrono::duration<double, std::milli> (end - start).count();

    return {containmentChecks, violationCount, milliseconds};
}


int main()
{
    constexpr double VIA_SIZE = 4.0;
    constexpr double ACTUAL_ENCLOSURE = 2.0;
    constexpr double GAP = 10.0;
    constexpr double MINIMUM_ENCLOSURE = 3.0;

    const std::vector<std::size_t> counts{100, 500, 1000, 2000};

    const std::filesystem::path outputPath = std::filesystem::path(DRCHECK_SOURCE_DIR) / "benchmarks" / "results" / "min_enclosure_benchmark.txt";
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

    printLine("MinEnclosure Benchmark");
    printLine("");

    for (const std::size_t count : counts)
    {
        const auto shapes = generateEnclosureGrid(count, VIA_SIZE, ACTUAL_ENCLOSURE, GAP);

        const auto brute = runBruteForce(shapes, MINIMUM_ENCLOSURE);
        // Calculate Index Build Time (Built once for all rules)
        const auto indexStart = std::chrono::steady_clock::now();
        LayerSpatialIndex spatialIndex(shapes);
        const auto indexEnd = std::chrono::steady_clock::now();
        const double indexBuildMilliseconds = std::chrono::duration<double, std::milli> (indexEnd - indexStart).count();

        const auto spatial = runSpatial(shapes, spatialIndex, MINIMUM_ENCLOSURE);

        if (brute.violationCount != spatial.violationCount)
        {
            throw std::runtime_error("Violation-count mismatch");
        }

        if (brute.violationCount != count)
        {
            throw std::runtime_error("Unexpected violation count");
        }

        const double reduction = brute.containmentChecks > 0 ? 
            100.0 * (1.0 - static_cast<double>(spatial.containmentChecks) / static_cast<double>(brute.containmentChecks)) : 0.0;

        const double speedup = spatial.milliseconds > 0.0 ? 
            brute.milliseconds / spatial.milliseconds : 0.0;

        printLine("-----------------");
        printLine("Via Count: " + std::to_string(count));
        printLine("Total Shapes: " + std::to_string(shapes.size()));
        printLine("Brute-Force Containment Checks: " + std::to_string(brute.containmentChecks));
        printLine("Spatial (QuadTree) Containment Checks: " + std::to_string(spatial.containmentChecks));
        printLine("Violation Count: " + std::to_string(spatial.violationCount));
        printLine("Check Reduction:  " + std::to_string(reduction) + "%");
        printLine("Brute Time: " + std::to_string(brute.milliseconds));
        printLine("Index Build Time: " + std::to_string(indexBuildMilliseconds));
        printLine("Spatial Rule Time:  " + std::to_string(spatial.milliseconds));
        printLine("Rule Speedup: " + std::to_string(speedup) + "x");
        printLine("");
    }

    return 0;
}