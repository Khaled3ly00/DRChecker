#include <gtest/gtest.h>

#include "drcheck/rules/RuleFactory.h"
#include "drcheck/rules/MinSpacingRule.h"
#include "drcheck/rules/MinWidthRule.h"
#include "drcheck/rules/MinEnclosureRule.h"
#include "drcheck/rules/DensityRule.h"
#include "drcheck/geometry/Constants.h"

using namespace drcheck;
using namespace drcheck::domain;
using namespace drcheck::rules;
using namespace drcheck::geometry;

TEST(RuleFactoryTest, CreatesMinSpacingRule)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    RuleParameters params;

    params.layer = M1;
    params.value = 0.25;

    auto rule = RuleFactory::create("min_spacing", params);

    const auto* minSpacingRule = dynamic_cast<const MinSpacingRule*>(rule.get());

    ASSERT_NE(minSpacingRule, nullptr);

    EXPECT_EQ(minSpacingRule->getLayer(), M1);
    EXPECT_NEAR(minSpacingRule->getMinimumSpacing(), 0.25, EPSILON);
}

TEST(RuleFactoryTest, CreatesMinWidthRule)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");

    RuleParameters params;

    params.layer =M1;
    params.value = 0.20;

    auto rule = RuleFactory::create("min_width", params);

    const auto* minWidthRule = dynamic_cast<const MinWidthRule*>(rule.get());

    ASSERT_NE(minWidthRule, nullptr);

    EXPECT_EQ(minWidthRule->getLayer(), M1);
    EXPECT_NEAR(minWidthRule->getMinimumWidth(), 0.20, EPSILON);
}

TEST(RuleFactoryTest, CreatesMinEnclosureRule)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    const Layer* VIA1 = registry.declare("VIA1");
    RuleParameters params;

    params.innerLayer = VIA1;
    params.outerLayer = M1;
    params.value = 1.0;

    auto rule = RuleFactory::create("min_enclosure", params);

    const auto* minEnclosureRule = dynamic_cast<const MinEnclosureRule*>(rule.get());

    ASSERT_NE(minEnclosureRule, nullptr);

    EXPECT_EQ(minEnclosureRule->getInnerLayer(), VIA1);
    EXPECT_EQ(minEnclosureRule->getOuterLayer(), M1);
    EXPECT_NEAR(minEnclosureRule->getMinimumEnclosure(), 1.0, EPSILON);
}

TEST(RuleFactoryTest, CreatesDensityRule)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    RuleParameters params;

    params.layer = M1;
    params.value = 0.30;
    params.densityLimit = DensityLimit::Minimum;
    params.windowSize = 10.0;
    params.windowStep = 5.0;
    params.analysisWindow = BoundingBox(0, 0, 100, 100);

    auto rule = RuleFactory::create("density", params);

    const auto* densityRule = dynamic_cast<const DensityRule*>(rule.get());

    ASSERT_NE(densityRule, nullptr);

    EXPECT_EQ(densityRule->getLayer(), M1);
    EXPECT_EQ(densityRule->getLimit(), DensityLimit::Minimum);
    EXPECT_NEAR(densityRule->getRequiredDensity(), 0.30, EPSILON);
    EXPECT_NEAR(densityRule->getWindowSize(), 10.0, EPSILON);
    EXPECT_NEAR(densityRule->getWindowStep(), 5.0, EPSILON);

    ASSERT_TRUE(densityRule->getAnalysisWindow().has_value());

    EXPECT_NEAR(densityRule->getAnalysisWindow()->getMinX(), 0.0, EPSILON);
    EXPECT_NEAR(densityRule->getAnalysisWindow()->getMaxX(), 100.0, EPSILON);
}

TEST(RuleFactoryTest, RejectsMissingMinSpacingValue)
{
    LayerRegistry registry;
    const Layer* M1 = registry.declare("M1");
    RuleParameters params;

    params.layer = M1;

    EXPECT_THROW(RuleFactory::create("min_spacing", params), std::invalid_argument);
}

TEST(RuleFactoryTest, RejectsUnknownRuleType)
{
    RuleParameters params;

    EXPECT_THROW(RuleFactory::create("unknown_rule", params), std::invalid_argument);
}


TEST(RuleFactoryTest, CreatesMinEnclosureRuleWithOptions)
{
    LayerRegistry registry;
    const Layer* CO = registry.declare("CO");
    const Layer* OD = registry.declare("OD");
    const Layer* PO = registry.declare("PO");
    RuleParameters params;

    params.innerLayer = CO;

    params.enclosureOptions = std::vector<EnclosureOption>{EnclosureOption(OD, 0.04, 0.00, 0.03), EnclosureOption(PO, 0.03)};

    auto rule = RuleFactory::create("min_enclosure", params);

    const auto* minEnclosureRule = dynamic_cast<const MinEnclosureRule*>(rule.get());

    ASSERT_NE(minEnclosureRule, nullptr);
}