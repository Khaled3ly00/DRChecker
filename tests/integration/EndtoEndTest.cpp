#include <gtest/gtest.h>
#include <filesystem>

#include "drcheck/io/JSONLayoutParser.h"
#include "drcheck/io/JSONRuleParser.h"
#include "drcheck/engine/DRCEngine.h"
#include "drcheck/io/JSONReportWriter.h"

using drcheck::engine::DRCEngine;
using drcheck::io::JSONLayoutParser;
using drcheck::io::JSONRuleParser;
using drcheck::io::JSONReportWriter;

TEST(EndToEndTest, ParsesFilesAndDetectsExpectedViolations)
{
    JSONLayoutParser layoutParser;
    JSONRuleParser ruleParser;
    DRCEngine engine;
    JSONReportWriter writer;

    const std::filesystem::path layout_path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "cli_multiple_shapes_layout.json";
    const auto shapes = layoutParser.load(layout_path.string());

    const std::filesystem::path rules_path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "cli_rules.json";
    const auto rules = ruleParser.load(rules_path.string());
    
    const auto violations = engine.run(shapes, rules);

    const std::filesystem::path report_path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "report.json";
    writer.write(violations, report_path.string());

    ASSERT_EQ(violations.size(), 3);
}