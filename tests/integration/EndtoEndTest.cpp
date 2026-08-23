#include <gtest/gtest.h>
#include <filesystem>

#include "drcheck/io/JSONLayoutParser.h"
#include "drcheck/io/JSONRuleParser.h"
#include "drcheck/engine/DRCEngine.h"
#include "drcheck/io/JSONReportWriter.h"
#include "drcheck/io/SVGReportWriter.h"
#include "drcheck/io/TclRuleParser.h"
#include "drcheck/geometry/Constants.h"

using drcheck::engine::DRCEngine;
using drcheck::io::JSONLayoutParser;
using drcheck::io::JSONRuleParser;
using drcheck::io::JSONReportWriter;
using drcheck::io::SVGReportWriter;
using drcheck::io::TclRuleParser;
using drcheck::geometry::EPSILON;

TEST(EndToEndTest, ParsesFilesAndDetectsExpectedViolations)
{
    const std::filesystem::path layout_path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "cli_multiple_shapes_layout.json";
    const auto shapes = JSONLayoutParser::load(layout_path.string());

    const std::filesystem::path rules_path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "cli_rules.json";
    const auto rules = JSONRuleParser::load(rules_path.string());
    
    const auto violations = DRCEngine::run(shapes, rules);

    const std::filesystem::path report_path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "report.json";
    JSONReportWriter::write(violations, report_path.string());

    const std::filesystem::path SVG_report_path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "report.svg";
    SVGReportWriter::write(shapes, violations, SVG_report_path.string());

    ASSERT_EQ(violations.size(), 5);
}


TEST(EndToEndTest, JsonAndTclRuleDecksProduceSameViolations)
{
    const std::string layoutPath = std::string(DRCHECK_SOURCE_DIR) + "/examples/cli_multiple_shapes_layout.json";
    const std::string jsonRulesPath = std::string(DRCHECK_SOURCE_DIR) + "/examples/equivalent_rules.json";
    const std::string tclRulesPath = std::string(DRCHECK_SOURCE_DIR) + "/examples/equivalent_rules.tcl";

    const auto shapes = JSONLayoutParser::load(layoutPath);

    const auto jsonRules = JSONRuleParser::load(jsonRulesPath);
    const auto tclRules = TclRuleParser::load(tclRulesPath);

    ASSERT_EQ(jsonRules.size(), tclRules.size());

    const auto jsonViolations = DRCEngine::run(shapes, jsonRules);
    const auto tclViolations = DRCEngine::run(shapes, tclRules);

    ASSERT_EQ(jsonViolations.size(), tclViolations.size());

    for (std::size_t i = 0; i < jsonViolations.size(); ++i)
    {
        EXPECT_EQ(jsonViolations[i].getType(), tclViolations[i].getType());
        EXPECT_EQ(jsonViolations[i].getShapeIds(), tclViolations[i].getShapeIds());
        EXPECT_NEAR(jsonViolations[i].getActualValue(), tclViolations[i].getActualValue(), EPSILON);
        EXPECT_NEAR(jsonViolations[i].getRequiredValue(), tclViolations[i].getRequiredValue(), EPSILON);
    }
}