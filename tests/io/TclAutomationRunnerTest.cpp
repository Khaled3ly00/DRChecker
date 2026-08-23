#include <gtest/gtest.h>
#include <filesystem>

#include "drcheck/io/TclAutomationRunner.h"

using drcheck::io::TclAutomationRunner;

TEST(TclAutomationRunnerTest, ExecutesDRCRunCommand)
{
    const std::string scriptPath = std::string(DRCHECK_SOURCE_DIR) + "/examples/automation.tcl";

    const auto violations = TclAutomationRunner::run(scriptPath);

    EXPECT_EQ(violations.size(), 5);
}

TEST(TclAutomationRunnerTest, ExecutesMultipleDRCRuns)
{
    const std::string scriptPath = std::string(DRCHECK_SOURCE_DIR) + "/examples/automation_multiple_runs.tcl";

    const auto violations = TclAutomationRunner::run(scriptPath);
    // second run violations
    EXPECT_EQ(violations.size(), 0);
    EXPECT_TRUE(std::filesystem::exists(std::string(DRCHECK_SOURCE_DIR) + "/examples/automation_report1.json"));
    EXPECT_TRUE(std::filesystem::exists(std::string(DRCHECK_SOURCE_DIR) + "/examples/automation_report2.json"));
}