#include <gtest/gtest.h>

#include "drcheck/engine/DRCRunner.h"

using drcheck::engine::DRCRunner;
using drcheck::engine::DRCRunConfig;

TEST(DRCRunnerTest, RunsDRCAndWritesReport)
{
    DRCRunConfig config;

    config.layoutPath =std::string(DRCHECK_SOURCE_DIR) + "/examples/cli_multiple_shapes_layout.json";

    config.rulesPath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";

    config.reportPath = std::string(DRCHECK_SOURCE_DIR) + "/build/test_report.json";

    const auto violations = DRCRunner::run(config);

    EXPECT_FALSE(violations.empty());
}