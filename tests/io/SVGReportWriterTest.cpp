#include <gtest/gtest.h>

#include "drcheck/io/SVGReportWriter.h"
#include "drcheck/geometry/Polygon.h"
#include "drcheck/domain/Layer.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <filesystem>

using drcheck::domain::Violation;
using drcheck::domain::ViolationType;
using drcheck::domain::ViolationMarker;
using drcheck::domain::Shape;
using drcheck::domain::Purpose;
using drcheck::domain::Layer;
using drcheck::geometry::Point;
using drcheck::geometry::Polygon;
using drcheck::io::SVGReportWriter;

const std::string outputPath = "test_output.svg";

TEST(SVGReportWriterTest, WritesPolygonToSVG)
{
    Polygon polygon({
        Point(0, 0),
        Point(10, 0),
        Point(10, 5),
        Point(0, 5)
        });

    Shape shape(1, Layer::M1, Purpose::Drawing, std::move(polygon));

    const std::vector<Shape> shapes{shape};
    const std::vector<Violation> violations;

    SVGReportWriter::write(shapes, violations, outputPath);

    std::ifstream input(outputPath);
    ASSERT_TRUE(input.is_open());

    std::stringstream buffer;
    buffer << input.rdbuf();

    const std::string content = buffer.str();

    EXPECT_NE(content.find("<svg"), std::string::npos);
    EXPECT_NE(content.find("<polygon"), std::string::npos);
    EXPECT_NE(content.find("</svg>"), std::string::npos);
    EXPECT_NE(content.find("fill=\"cyan\""), std::string::npos);
    EXPECT_NE(content.find("stroke=\"cyan\""), std::string::npos);
    EXPECT_NE(content.find("fill-opacity=\"0.35\""), std::string::npos);
    // Check for shape ID
    EXPECT_NE(content.find("<text"), std::string::npos);
    EXPECT_NE(content.find("1"), std::string::npos);

    input.close();
    std::remove(outputPath.c_str());
}

TEST(SVGReportWriterTest, WritesViolationToSVG)
{
    Polygon polygon({
    Point(0, 0),
    Point(10, 0),
    Point(10, 5),
    Point(0, 5)
        });

    Shape shape(1, Layer::M1, Purpose::Drawing, std::move(polygon));
    ViolationMarker marker{Point(4, 2), Point(7, 2), 1, 3};
    Violation violation(ViolationType::MinSpacing,{ 1, 2 }, "Minimum spacing violation", 3.0, 4.0, marker);

    const std::vector<Shape> shapes{ shape };
    const std::vector<Violation> violations{ violation };

    SVGReportWriter::write(shapes, violations, outputPath);

    std::ifstream input(outputPath);
    ASSERT_TRUE(input.is_open());

    std::stringstream buffer;
    buffer << input.rdbuf();

    const std::string content = buffer.str();

    EXPECT_NE(content.find("<line"), std::string::npos);
    EXPECT_NE(content.find("stroke=\"red\""), std::string::npos);

    input.close();
    std::remove(outputPath.c_str());
}