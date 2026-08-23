#include <gtest/gtest.h>

#include "drcheck/io/TclRuleParser.h"
#include "drcheck/rules/MinSpacingRule.h"
#include "drcheck/rules/MinWidthRule.h"
#include "drcheck/rules/MinEnclosureRule.h"
#include "drcheck/rules/DensityRule.h"
#include "drcheck/geometry/Constants.h"
#include "drcheck/domain/Layer.h"

using drcheck::io::TclRuleParser;
using drcheck::rules::MinSpacingRule;
using drcheck::rules::MinWidthRule;
using drcheck::rules::MinEnclosureRule;
using drcheck::rules::DensityRule;
using drcheck::rules::DensityLimit;
using drcheck::geometry::EPSILON;
using drcheck::domain::Layer;

TEST(TclRuleParserTest, ParsesMinSpacingRule)
{
    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath);

    const auto* minSpacingRule = dynamic_cast<const MinSpacingRule*>(rules[0].get());

    ASSERT_NE(minSpacingRule, nullptr);

    EXPECT_EQ(minSpacingRule->getLayer(), Layer::Metal1);
    EXPECT_NEAR(minSpacingRule->getMinimumSpacing(), 0.25, EPSILON);
}


TEST(TclRuleParserTest, ParsesMinSpacingOptionsInAnyOrder)
{
    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath);

    const auto* minSpacingRule = dynamic_cast<const MinSpacingRule*>(rules[1].get());

    ASSERT_NE(minSpacingRule, nullptr);

    EXPECT_EQ(minSpacingRule->getLayer(), Layer::Metal2);
    EXPECT_NEAR(minSpacingRule->getMinimumSpacing(), 0.25, EPSILON);
}

TEST(TclRuleParserTest, ThrowsOnInvalidRule)
{
    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/invalid_rules.tcl";
    EXPECT_THROW(TclRuleParser::load(filePath), std::invalid_argument);
}

TEST(TclRuleParserTest, ParsesMinWidthRule)
{
    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath);

    const auto* minWidthRule = dynamic_cast<const MinWidthRule*>(rules[2].get());

    ASSERT_NE(minWidthRule, nullptr);

    EXPECT_EQ(minWidthRule->getLayer(), Layer::Metal1);
    EXPECT_NEAR(minWidthRule->getMinimumWidth(), 0.20, EPSILON);
}

TEST(TclRuleParserTest, ParsesMinEnclosureRule)
{
    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath);

    const auto* minEnclosureRule = dynamic_cast<const MinEnclosureRule*>(rules[3].get());

    ASSERT_NE(minEnclosureRule, nullptr);

    EXPECT_EQ(minEnclosureRule->getInnerLayer(), Layer::Via12);
    EXPECT_EQ(minEnclosureRule->getOuterLayer(), Layer::Metal1);
    EXPECT_NEAR(minEnclosureRule->getMinimumEnclosure(), 0.10, EPSILON);
}

TEST(TclRuleParserTest, ParsesMinimumDensityRule)
{
    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath);

    const auto* densityRule = dynamic_cast<const DensityRule*>(rules[4].get());

    ASSERT_NE(densityRule, nullptr);

    EXPECT_EQ(densityRule->getLayer(), Layer::Metal1);
    EXPECT_EQ(densityRule->getLimit(), DensityLimit::Minimum);
    EXPECT_NEAR(densityRule->getRequiredDensity(), 0.30, EPSILON);
    EXPECT_NEAR(densityRule->getWindowSize(), 10.0, EPSILON);
    EXPECT_NEAR(densityRule->getWindowStep(), 5.0, EPSILON);
    EXPECT_FALSE(densityRule->getAnalysisWindow().has_value());
}

TEST(TclRuleParserTest, ParsesMaximumDensityRule)
{
    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath);

    const auto* densityRule = dynamic_cast<const DensityRule*>(rules[5].get());

    ASSERT_NE(densityRule, nullptr);

    EXPECT_EQ(densityRule->getLayer(), Layer::Metal2);
    EXPECT_EQ(densityRule->getLimit(), DensityLimit::Maximum);
    EXPECT_NEAR(densityRule->getRequiredDensity(), 0.70, EPSILON);
    EXPECT_NEAR(densityRule->getWindowSize(), 20.0, EPSILON);
    EXPECT_NEAR(densityRule->getWindowStep(), 10.0, EPSILON);
    EXPECT_FALSE(densityRule->getAnalysisWindow().has_value());
}

TEST(TclRuleParserTest, ParsesDensityRuleWithAnalysisRegion)
{
    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath);

    const auto* densityRule = dynamic_cast<const DensityRule*>(rules[6].get());

    ASSERT_NE(densityRule, nullptr);

    EXPECT_EQ(densityRule->getLayer(), Layer::Metal2);
    EXPECT_EQ(densityRule->getLimit(), DensityLimit::Maximum);
    EXPECT_NEAR(densityRule->getRequiredDensity(), 0.70, EPSILON);
    EXPECT_NEAR(densityRule->getWindowSize(), 20.0, EPSILON);
    EXPECT_NEAR(densityRule->getWindowStep(), 10.0, EPSILON);

    ASSERT_TRUE(densityRule->getAnalysisWindow().has_value());

    const auto& region = densityRule->getAnalysisWindow().value();

    EXPECT_NEAR(region.getMinX(), 0.0, EPSILON);
    EXPECT_NEAR(region.getMinY(), 0.0, EPSILON);
    EXPECT_NEAR(region.getMaxX(), 100.0, EPSILON);
    EXPECT_NEAR(region.getMaxY(), 100.0, EPSILON);
}