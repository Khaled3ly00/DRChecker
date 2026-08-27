#include <gtest/gtest.h>

#include "drcheck/io/JSONRuleParser.h"
#include "drcheck/rules/MinSpacingRule.h"
#include "drcheck/rules/MinWidthRule.h"
#include "drcheck/rules/MinEnclosureRule.h"
#include "drcheck/rules/DensityRule.h"

#include <filesystem>

using namespace drcheck::rules;
using drcheck::domain::Layer;
using drcheck::io::JSONRuleParser;

const std::filesystem::path path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "rules.json";
JSONRuleParser parser;
const auto rules = parser.load(path.string());

TEST(JSONRuleParserTest, ParsingRulesFromJSONTestingMinWidthRuleM1) {
    ASSERT_EQ(rules.size(), 8);
    // EXPECT_EQ(rules[0]->getMinimumWidth(), 3); Doesn't work (compile time error) use dynamic casting instead
    // cast polymorhic rule pointer to MinWidthRule pointer
    const auto* minWidthRule = dynamic_cast<const MinWidthRule*>(rules[0].get());
    ASSERT_NE(minWidthRule, nullptr);
    EXPECT_DOUBLE_EQ(minWidthRule->getMinimumWidth(), 3.0);
    EXPECT_EQ(minWidthRule->getLayer(), Layer::M1);
}

TEST(JSONRuleParserTest, ParsingRulesFromJSONTestingMinWidthRuleM2) {
    ASSERT_EQ(rules.size(), 8);
    // cast polymorhic rule pointer to MinWidthRule pointer
    const auto* minWidthRule = dynamic_cast<const MinWidthRule*>(rules[1].get());
    ASSERT_NE(minWidthRule, nullptr);
    EXPECT_DOUBLE_EQ(minWidthRule->getMinimumWidth(), 4.0);
    EXPECT_EQ(minWidthRule->getLayer(), Layer::M2);
}

TEST(JSONRuleParserTest, ParsingRulesFromJSONTestingMinWidthRuleVIA12) {
    ASSERT_EQ(rules.size(), 8);
    // cast polymorhic rule pointer to MinWidthRule pointer
    const auto* minWidthRule = dynamic_cast<const MinWidthRule*>(rules[2].get());
    ASSERT_NE(minWidthRule, nullptr);
    EXPECT_DOUBLE_EQ(minWidthRule->getMinimumWidth(), 1.0);
    EXPECT_EQ(minWidthRule->getLayer(), Layer::VIA1);
}

TEST(JSONRuleParserTest, ParsingRulesFromJSONTestingMinSpacingRuleM1) {
    ASSERT_EQ(rules.size(), 8);
    // cast polymorhic rule pointer to MinWidthRule pointer
    const auto* minSpacingRule = dynamic_cast<const MinSpacingRule*>(rules[3].get());
    ASSERT_NE(minSpacingRule, nullptr);
    EXPECT_DOUBLE_EQ(minSpacingRule->getMinimumSpacing(), 2.0);
    EXPECT_EQ(minSpacingRule->getLayer(), Layer::M1);
}

TEST(JSONRuleParserTest, ParsingRulesFromJSONTestingMinEnclosureRuleVIA12M1) {
    ASSERT_EQ(rules.size(), 8);
    // cast polymorhic rule pointer to MinWidthRule pointer
    const auto* minEnclosureRule = dynamic_cast<const MinEnclosureRule*>(rules[4].get());
    ASSERT_NE(minEnclosureRule, nullptr);
    EXPECT_DOUBLE_EQ(minEnclosureRule->getMinimumEnclosure(), 2.0);
    EXPECT_EQ(minEnclosureRule->getInnerLayer(), Layer::VIA1);
    EXPECT_EQ(minEnclosureRule->getOuterLayer(), Layer::M1);
}

TEST(JSONRuleParserTest, ParsingRulesFromJSONTestingInvalidRule) {
    const std::filesystem::path path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "invalid_rules.json";
    JSONRuleParser parser;
    EXPECT_THROW(parser.load(path.string()), std::invalid_argument);
}

TEST(JSONRuleParserTest, ParsesMinimumDensityRule)
{
    const auto* densityRule = dynamic_cast<const DensityRule*>(rules[5].get());

    ASSERT_NE(densityRule, nullptr);

    EXPECT_EQ(densityRule->getLayer(), Layer::M1);
    EXPECT_EQ(densityRule->getLimit(), DensityLimit::Minimum);
    EXPECT_DOUBLE_EQ(densityRule->getRequiredDensity(), 0.30);
    EXPECT_DOUBLE_EQ(densityRule->getWindowSize(), 10.0);
    EXPECT_DOUBLE_EQ(densityRule->getWindowStep(), 5.0);

    EXPECT_FALSE(densityRule->getAnalysisWindow().has_value());
}


TEST(JSONRuleParserTest, ParsesMaximummDensityRule)
{
    const auto* densityRule = dynamic_cast<const DensityRule*>(rules[6].get());

    ASSERT_NE(densityRule, nullptr);

    EXPECT_EQ(densityRule->getLayer(), Layer::M2);
    EXPECT_EQ(densityRule->getLimit(), DensityLimit::Maximum);
    EXPECT_DOUBLE_EQ(densityRule->getRequiredDensity(), 0.70);
    EXPECT_DOUBLE_EQ(densityRule->getWindowSize(), 20.0);
    EXPECT_DOUBLE_EQ(densityRule->getWindowStep(), 10.0);

    EXPECT_FALSE(densityRule->getAnalysisWindow().has_value());
}

TEST(JSONRuleParserTest, ParsesDensityRuleWithExplicitWindow)
{
    const auto* densityRule = dynamic_cast<const DensityRule*>(rules[7].get());

    ASSERT_NE(densityRule, nullptr);

    ASSERT_TRUE(densityRule->getAnalysisWindow().has_value());

    const auto& analysisWindow = densityRule->getAnalysisWindow().value();

    EXPECT_DOUBLE_EQ(analysisWindow.getMinX(), 10.0);
    EXPECT_DOUBLE_EQ(analysisWindow.getMinY(), 20.0);
    EXPECT_DOUBLE_EQ(analysisWindow.getMaxX(), 50.0);
    EXPECT_DOUBLE_EQ(analysisWindow.getMaxY(), 70.0);
}