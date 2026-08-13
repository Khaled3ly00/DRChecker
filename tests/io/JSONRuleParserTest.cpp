#include <gtest/gtest.h>

#include "drcheck/io/JSONRuleParser.h"

#include <filesystem>

using namespace drcheck::rules;
using drcheck::domain::Layer;
using drcheck::io::JSONRuleParser;

const std::filesystem::path path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "rules.json";
JSONRuleParser parser;
const auto rules = parser.load(path.string());

TEST(JSONRuleParserTest, ParsingRulesFromJSONTestingMinWidthRuleM1) {
    ASSERT_EQ(rules.size(), 5);
    // EXPECT_EQ(rules[0]->getMinimumWidth(), 3); Doesn't work (compile time error) use dynamic casting instead
    // cast polymorhic rule pointer to MinWidthRule pointer
    const auto* minWidthRule = dynamic_cast<const MinWidthRule*>(rules[0].get());
    ASSERT_NE(minWidthRule, nullptr);
    EXPECT_DOUBLE_EQ(minWidthRule->getMinimumWidth(), 3.0);
    EXPECT_EQ(minWidthRule->getLayer(), Layer::Metal1);
}

TEST(JSONRuleParserTest, ParsingRulesFromJSONTestingMinWidthRuleM2) {
    ASSERT_EQ(rules.size(), 5);
    // cast polymorhic rule pointer to MinWidthRule pointer
    const auto* minWidthRule = dynamic_cast<const MinWidthRule*>(rules[1].get());
    ASSERT_NE(minWidthRule, nullptr);
    EXPECT_DOUBLE_EQ(minWidthRule->getMinimumWidth(), 4.0);
    EXPECT_EQ(minWidthRule->getLayer(), Layer::Metal2);
}

TEST(JSONRuleParserTest, ParsingRulesFromJSONTestingMinWidthRuleVIA12) {
    ASSERT_EQ(rules.size(), 5);
    // cast polymorhic rule pointer to MinWidthRule pointer
    const auto* minWidthRule = dynamic_cast<const MinWidthRule*>(rules[2].get());
    ASSERT_NE(minWidthRule, nullptr);
    EXPECT_DOUBLE_EQ(minWidthRule->getMinimumWidth(), 1.0);
    EXPECT_EQ(minWidthRule->getLayer(), Layer::Via12);
}

TEST(JSONRuleParserTest, ParsingRulesFromJSONTestingMinSpacingRuleM1) {
    ASSERT_EQ(rules.size(), 5);
    // cast polymorhic rule pointer to MinWidthRule pointer
    const auto* minSpacingRule = dynamic_cast<const MinSpacingRule*>(rules[3].get());
    ASSERT_NE(minSpacingRule, nullptr);
    EXPECT_DOUBLE_EQ(minSpacingRule->getMinimumSpacing(), 2.0);
    EXPECT_EQ(minSpacingRule->getLayer(), Layer::Metal1);
}

TEST(JSONRuleParserTest, ParsingRulesFromJSONTestingMinEnclosureRuleVIA12M1) {
    ASSERT_EQ(rules.size(), 5);
    // cast polymorhic rule pointer to MinWidthRule pointer
    const auto* minEnclosureRule = dynamic_cast<const MinEnclosureRule*>(rules[4].get());
    ASSERT_NE(minEnclosureRule, nullptr);
    EXPECT_DOUBLE_EQ(minEnclosureRule->getMinimumEnclosure(), 2.0);
    EXPECT_EQ(minEnclosureRule->getInnerLayer(), Layer::Via12);
    EXPECT_EQ(minEnclosureRule->getOuterLayer(), Layer::Metal1);
}

TEST(JSONRuleParserTest, ParsingRulesFromJSONTestingInvalidRule) {
    const std::filesystem::path path = std::filesystem::path(DRCHECK_SOURCE_DIR) / "examples" / "invalid_rules.json";
    JSONRuleParser parser;
    EXPECT_THROW(parser.load(path.string()), std::invalid_argument);
}