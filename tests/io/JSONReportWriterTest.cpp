#include <gtest/gtest.h>

#include "drcheck/io/JSONReportWriter.h"
#include "drcheck/geometry/BoundingBox.h"

#include <nlohmann/json.hpp>
#include <cstdio>
#include <fstream>
#include <string>
#include <filesystem>

using drcheck::domain::Violation;
using drcheck::domain::ViolationType;
using drcheck::domain::ViolationMarker;
using drcheck::geometry::Point;
using drcheck::io::JSONReportWriter;

TEST(JSONReportWriterTest, CreatingSingleViolationReport) {
    std::vector<Violation> violations;
    ViolationMarker marker{
        Point(4.0, 2.0),
        Point(4.0, 5.0),
        2,
        0
    };
    violations.emplace_back(ViolationType::MinWidth, std::vector<std::size_t>{7}, "Minimum width violation", 2.0, 3.0, marker);

    const std::string outputPath = "json_report_writer_test.json";

    JSONReportWriter writer;
    writer.write(violations, outputPath);

    std::ifstream input(outputPath);

    ASSERT_TRUE(input.is_open());

    nlohmann::json json;
    input >> json;

    EXPECT_EQ(json["violationCount"].get<std::size_t>(), 1);
    ASSERT_TRUE(json["violations"].is_array());
    ASSERT_EQ(json["violations"].size(), 1);
    EXPECT_EQ(json["violations"][0]["type"].get<std::string>(), "MinWidth");
    ASSERT_TRUE(json["violations"][0]["shapeIds"].is_array());
    ASSERT_EQ(json["violations"][0]["shapeIds"].size(), 1);
    EXPECT_EQ(json["violations"][0]["shapeIds"][0].get<std::size_t>(), 7);
    EXPECT_EQ(json["violations"][0]["message"].get<std::string>(), "Minimum width violation");
    EXPECT_DOUBLE_EQ(json["violations"][0]["actual"], 2.0);
    EXPECT_DOUBLE_EQ(json["violations"][0]["required"], 3.0);
    EXPECT_DOUBLE_EQ(json["violations"][0]["marker"]["firstPoint"]["x"], 4.0);
    EXPECT_DOUBLE_EQ(json["violations"][0]["marker"]["firstPoint"]["y"], 2.0);
    EXPECT_DOUBLE_EQ(json["violations"][0]["marker"]["secondPoint"]["x"], 4.0);
    EXPECT_DOUBLE_EQ(json["violations"][0]["marker"]["secondPoint"]["y"], 5.0);

    input.close();
    std::remove(outputPath.c_str());
}

TEST(JSONReportWriterTest, CreatingEmptyViolationReport) {
    std::vector<Violation> violations;

    const std::string outputPath = "json_report_writer_test.json";

    JSONReportWriter writer;
    writer.write(violations, outputPath);

    std::ifstream input(outputPath);

    ASSERT_TRUE(input.is_open());

    nlohmann::json json;
    input >> json;

    EXPECT_EQ(json["violationCount"].get<std::size_t>(), 0);
    ASSERT_TRUE(json["violations"].is_array());
    EXPECT_EQ(json["violations"].size(), 0);

    input.close();
    std::remove(outputPath.c_str());
}

TEST(JSONReportWriterTest, CreatingMultipleViolationReport) {
    std::vector<Violation> violations;

    violations.emplace_back(ViolationType::MinWidth, std::vector<std::size_t>{7}, "Minimum width violation", 2.0, 3.0);
    violations.emplace_back(ViolationType::Enclosure, std::vector<std::size_t>{7, 10}, "Minimum enclosure violation", 2.0, 3.0);

    const std::string outputPath = "json_report_writer_test.json";

    JSONReportWriter writer;
    writer.write(violations, outputPath);

    std::ifstream input(outputPath);

    ASSERT_TRUE(input.is_open());

    nlohmann::json json;
    input >> json;

    EXPECT_EQ(json["violationCount"].get<std::size_t>(), 2);
    ASSERT_TRUE(json["violations"].is_array());
    ASSERT_EQ(json["violations"].size(), 2);
    EXPECT_EQ(json["violations"][0]["type"].get<std::string>(), "MinWidth");
    ASSERT_TRUE(json["violations"][0]["shapeIds"].is_array());
    ASSERT_EQ(json["violations"][0]["shapeIds"].size(), 1);
    EXPECT_EQ(json["violations"][0]["shapeIds"][0].get<std::size_t>(), 7);
    EXPECT_EQ(json["violations"][0]["message"].get<std::string>(), "Minimum width violation");
    EXPECT_EQ(json["violations"][1]["type"].get<std::string>(), "Enclosure");
    ASSERT_TRUE(json["violations"][1]["shapeIds"].is_array());
    ASSERT_EQ(json["violations"][1]["shapeIds"].size(), 2);
    EXPECT_EQ(json["violations"][1]["shapeIds"][0].get<std::size_t>(), 7);
    EXPECT_EQ(json["violations"][1]["shapeIds"][1].get<std::size_t>(), 10);
    EXPECT_EQ(json["violations"][1]["message"].get<std::string>(), "Minimum enclosure violation");
    EXPECT_FALSE(json["violations"][0].contains("marker"));
    EXPECT_FALSE(json["violations"][1].contains("marker"));
    input.close();
    std::remove(outputPath.c_str());
}

TEST(JSONReportWriterTest, CreatesReigonMarker) {
    std::vector<Violation> violations;
    ViolationMarker marker{
    .region = drcheck::geometry::BoundingBox(20, 0, 23, 10)
    };
    violations.emplace_back(ViolationType::MinDensity, std::vector<std::size_t>{}, "Minimum Density violation", 0.3, 0.7, marker);

    const std::string outputPath = "json_report_writer_test.json";

    JSONReportWriter writer;
    writer.write(violations, outputPath);

    std::ifstream input(outputPath);

    ASSERT_TRUE(input.is_open());

    nlohmann::json json;
    input >> json;

    ASSERT_TRUE(json["violations"][0].contains("marker"));
    ASSERT_TRUE(json["violations"][0]["marker"].contains("region"));
    EXPECT_EQ(json["violations"][0]["marker"]["region"]["minX"], 20);
    EXPECT_EQ(json["violations"][0]["marker"]["region"]["maxX"], 23);
    EXPECT_FALSE(json["violations"][0]["marker"].contains("firstPoint"));
    input.close();
    std::remove(outputPath.c_str());
}