#include <gtest/gtest.h>
#include <stdexcept>

#include "drcheck/domain/LayerRegistry.h"

using drcheck::domain::Layer;
using drcheck::domain::LayerRegistry;

TEST(LayerRegistryTest, DeclaresAndResolvesLayer)
{
    LayerRegistry registry;

    registry.declare("M1");

    const Layer* layer = registry.resolve("M1");

    EXPECT_EQ(layer->getName(), "M1");
}

TEST(LayerRegistryTest, ResolveReturnsCanonicalLayer)
{
    LayerRegistry registry;

    const Layer* declared = registry.declare("M1");
    const Layer* resolved = registry.resolve("M1");

    EXPECT_EQ(declared, resolved);
}

TEST(LayerRegistryTest, RepeatedResolveReturnsSameLayer)
{
    LayerRegistry registry;

    registry.declare("M1");

    EXPECT_EQ(registry.resolve("M1"), registry.resolve("M1"));
}

TEST(LayerRegistryTest, DifferentLayersHaveDifferentIdentity)
{
    LayerRegistry registry;

    registry.declare("M1");
    registry.declare("M2");

    EXPECT_NE(registry.resolve("M1"), registry.resolve("M2"));
}

TEST(LayerRegistryTest, RejectsDuplicateDeclaration)
{
    LayerRegistry registry;

    registry.declare("M1");

    EXPECT_THROW(registry.declare("M1"), std::invalid_argument);
}

TEST(LayerRegistryTest, RejectsUndeclaredLayer)
{
    LayerRegistry registry;

    EXPECT_THROW(registry.resolve("M1"), std::invalid_argument);
}

TEST(LayerRegistryTest, RejectsEmptyLayerName)
{
    LayerRegistry registry;

    EXPECT_THROW(registry.declare(""), std::invalid_argument);
}