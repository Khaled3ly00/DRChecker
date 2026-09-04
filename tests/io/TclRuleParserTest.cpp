#include <gtest/gtest.h>

#include "drcheck/io/TclRuleParser.h"
#include "drcheck/rules/MinSpacingRule.h"
#include "drcheck/rules/MinWidthRule.h"
#include "drcheck/rules/MinEnclosureRule.h"
#include "drcheck/rules/DensityRule.h"
#include "drcheck/geometry/Constants.h"
#include "drcheck/domain/Layer.h"
#include "drcheck/domain/LayerRegistry.h"

using drcheck::io::TclRuleParser;
using drcheck::rules::MinSpacingRule;
using drcheck::rules::MinWidthRule;
using drcheck::rules::MinEnclosureRule;
using drcheck::rules::DensityRule;
using drcheck::rules::DensityLimit;
using drcheck::geometry::EPSILON;
using drcheck::domain::Layer;
using drcheck::domain::LayerRegistry;

TEST(TclRuleParserTest, ParsesMinSpacingRule)
{
	LayerRegistry registry;

    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath, registry);

    const auto* minSpacingRule = dynamic_cast<const MinSpacingRule*>(rules[0].get());

    ASSERT_NE(minSpacingRule, nullptr);

    EXPECT_EQ(minSpacingRule->getLayer(), registry.resolve("M1"));
    EXPECT_NEAR(minSpacingRule->getMinimumSpacing(), 0.25, EPSILON);
    EXPECT_EQ(registry.resolveGDS(15, 0), registry.resolve("M1"));
    EXPECT_EQ(registry.resolveGDS(15, 1), registry.resolve("M1"));
}


TEST(TclRuleParserTest, ParsesMinSpacingOptionsInAnyOrder)
{
    LayerRegistry registry;

    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath, registry);

    const auto* minSpacingRule = dynamic_cast<const MinSpacingRule*>(rules[1].get());

    ASSERT_NE(minSpacingRule, nullptr);

    EXPECT_EQ(minSpacingRule->getLayer(), registry.resolve("M2"));
    EXPECT_NEAR(minSpacingRule->getMinimumSpacing(), 0.25, EPSILON);
    EXPECT_EQ(registry.resolveGDS(16, 1), registry.resolve("M2"));
}

TEST(TclRuleParserTest, ThrowsOnInvalidRuleOrLayer)
{
    LayerRegistry registry;

    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/invalid_rules.tcl";
    EXPECT_THROW(TclRuleParser::load(filePath, registry), std::invalid_argument);
}

TEST(TclRuleParserTest, ParsesMinWidthRule)
{
    LayerRegistry registry;

    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath, registry);

    const auto* minWidthRule = dynamic_cast<const MinWidthRule*>(rules[2].get());

    ASSERT_NE(minWidthRule, nullptr);

    EXPECT_EQ(minWidthRule->getLayer(), registry.resolve("M1"));
    EXPECT_NEAR(minWidthRule->getMinimumWidth(), 0.20, EPSILON);
}

TEST(TclRuleParserTest, ParsesMinEnclosureRule)
{
    LayerRegistry registry;

    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath, registry);

    const auto* minEnclosureRule = dynamic_cast<const MinEnclosureRule*>(rules[3].get());

    ASSERT_NE(minEnclosureRule, nullptr);

    EXPECT_EQ(minEnclosureRule->getInnerLayer(), registry.resolve("VIA1"));
    EXPECT_EQ(minEnclosureRule->getOuterLayer(), registry.resolve("M1"));
    EXPECT_NEAR(minEnclosureRule->getMinimumEnclosure(), 0.10, EPSILON);
}

TEST(TclRuleParserTest, ParsesMinimumDensityRule)
{
    LayerRegistry registry;

    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath, registry);

    const auto* densityRule = dynamic_cast<const DensityRule*>(rules[4].get());

    ASSERT_NE(densityRule, nullptr);

    EXPECT_EQ(densityRule->getLayer(), registry.resolve("M1"));
    EXPECT_EQ(densityRule->getLimit(), DensityLimit::Minimum);
    EXPECT_NEAR(densityRule->getRequiredDensity(), 0.30, EPSILON);
    EXPECT_NEAR(densityRule->getWindowSize(), 10.0, EPSILON);
    EXPECT_NEAR(densityRule->getWindowStep(), 5.0, EPSILON);
    EXPECT_FALSE(densityRule->getAnalysisWindow().has_value());
}

TEST(TclRuleParserTest, ParsesMaximumDensityRule)
{
    LayerRegistry registry;

    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath, registry);

    const auto* densityRule = dynamic_cast<const DensityRule*>(rules[5].get());

    ASSERT_NE(densityRule, nullptr);

    EXPECT_EQ(densityRule->getLayer(), registry.resolve("M2"));
    EXPECT_EQ(densityRule->getLimit(), DensityLimit::Maximum);
    EXPECT_NEAR(densityRule->getRequiredDensity(), 0.70, EPSILON);
    EXPECT_NEAR(densityRule->getWindowSize(), 20.0, EPSILON);
    EXPECT_NEAR(densityRule->getWindowStep(), 10.0, EPSILON);
    EXPECT_FALSE(densityRule->getAnalysisWindow().has_value());
}

TEST(TclRuleParserTest, ParsesDensityRuleWithAnalysisRegion)
{
    LayerRegistry registry;

    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath, registry);

    const auto* densityRule = dynamic_cast<const DensityRule*>(rules[6].get());

    ASSERT_NE(densityRule, nullptr);

    EXPECT_EQ(densityRule->getLayer(), registry.resolve("M2"));
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

TEST(TclRuleParserTest, ParsesEnclosureRuleWithOneSimpleOption)
{
    LayerRegistry registry;

    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath, registry);

    const auto* minEnclosureRule = dynamic_cast<const MinEnclosureRule*>(rules[7].get());

    ASSERT_NE(minEnclosureRule, nullptr);

    EXPECT_EQ(minEnclosureRule->getInnerLayer(), registry.resolve("CO"));

    const auto& option = minEnclosureRule->getEnclosureOption(0);
    EXPECT_EQ(option.getOuterLayer(), registry.resolve("M1"));
    EXPECT_DOUBLE_EQ(option.getAllSidesMinEnclosure(), 0.04);
}

TEST(TclRuleParserTest, ParsesEnclosureRuleWithTwoSimpleOptions)
{
    LayerRegistry registry;

    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath, registry);

    const auto* minEnclosureRule = dynamic_cast<const MinEnclosureRule*>(rules[8].get());
    ASSERT_NE(minEnclosureRule, nullptr);

    EXPECT_EQ(minEnclosureRule->getInnerLayer(), registry.resolve("CO"));

    const auto& option0 = minEnclosureRule->getEnclosureOption(0);
    EXPECT_EQ(option0.getOuterLayer(), registry.resolve("M1"));
    EXPECT_DOUBLE_EQ(option0.getAllSidesMinEnclosure(), 0.04);

    const auto& option1 = minEnclosureRule->getEnclosureOption(1);
    EXPECT_EQ(option1.getOuterLayer(), registry.resolve("PO"));
    EXPECT_DOUBLE_EQ(option1.getAllSidesMinEnclosure(), 0.03);
}

TEST(TclRuleParserTest, ParsesEnclosureRuleWithTwoComplexOptions)
{
    LayerRegistry registry;

    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath, registry);

    const auto* minEnclosureRule = dynamic_cast<const MinEnclosureRule*>(rules[9].get());
    ASSERT_NE(minEnclosureRule, nullptr);

    EXPECT_EQ(minEnclosureRule->getInnerLayer(), registry.resolve("CO"));

    const auto& option0 = minEnclosureRule->getEnclosureOption(0);
    EXPECT_EQ(option0.getOuterLayer(), registry.resolve("M1"));
    EXPECT_DOUBLE_EQ(option0.getAllSidesMinEnclosure(), 0.04);
    EXPECT_DOUBLE_EQ(option0.getFirstPairMinEnclosure(), 0.00);
    EXPECT_DOUBLE_EQ(option0.getSecondPairMinEnclosure(), 0.06);

    const auto& option1 = minEnclosureRule->getEnclosureOption(1);
    EXPECT_EQ(option1.getOuterLayer(), registry.resolve("PO"));
    EXPECT_DOUBLE_EQ(option1.getAllSidesMinEnclosure(), 0.04);
    EXPECT_DOUBLE_EQ(option1.getFirstPairMinEnclosure(), 0.00);
    EXPECT_DOUBLE_EQ(option1.getSecondPairMinEnclosure(), 0.08);
}

TEST(TclRuleParserTest, ParsesEnclosureRuleWithOneSimpleOneComplexOptions)
{
    LayerRegistry registry;
    const std::string filePath = std::string(DRCHECK_SOURCE_DIR) + "/examples/rules.tcl";
    const auto rules = TclRuleParser::load(filePath, registry);

    const auto* minEnclosureRule = dynamic_cast<const MinEnclosureRule*>(rules[10].get());
    ASSERT_NE(minEnclosureRule, nullptr);

    EXPECT_EQ(minEnclosureRule->getInnerLayer(), registry.resolve("CO"));

    const auto& option0 = minEnclosureRule->getEnclosureOption(0);
    EXPECT_EQ(option0.getOuterLayer(), registry.resolve("M1"));
    EXPECT_DOUBLE_EQ(option0.getAllSidesMinEnclosure(), 0.04);
    EXPECT_DOUBLE_EQ(option0.getFirstPairMinEnclosure(), 0.00);
    EXPECT_DOUBLE_EQ(option0.getSecondPairMinEnclosure(), 0.06);

    const auto& option1 = minEnclosureRule->getEnclosureOption(1);
    EXPECT_EQ(option1.getOuterLayer(), registry.resolve("PO"));
    EXPECT_DOUBLE_EQ(option1.getAllSidesMinEnclosure(), 0.04);
}