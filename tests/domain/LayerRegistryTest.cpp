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

TEST(LayerRegistryTest, ResolvesGDSMapping)
{
    LayerRegistry registry;

    const Layer* M1 = registry.declare("M1");

    registry.mapGDS(M1, 15, 0);

    EXPECT_EQ(registry.resolveGDS(15, 0), M1);
}

TEST(LayerRegistryTest, AllowsMultipleGDSPairsForSameLayer)
{
    LayerRegistry registry;

    const Layer* M1 = registry.declare("M1");

    registry.mapGDS(M1, 15, 0);
    registry.mapGDS(M1, 15, 1);

    EXPECT_EQ(registry.resolveGDS(15, 0), M1);
    EXPECT_EQ(registry.resolveGDS(15, 1), M1);
}

TEST(LayerRegistryTest, RejectsMultipleLayersForSameGDSPair)
{
    LayerRegistry registry;

    const Layer* M1 = registry.declare("M1");
    const Layer* M2 = registry.declare("M2");

    registry.mapGDS(M1, 15, 0);

    EXPECT_THROW(registry.mapGDS(M2, 15, 0), std::invalid_argument);
}

TEST(LayerRegistryTest, RejectsUnmappedGDSPair)
{
    LayerRegistry registry;

    EXPECT_THROW(registry.resolveGDS(99, 0), std::invalid_argument);
}

TEST(LayerRegistryTest, RejectsLayerFromDifferentRegistry)
{
    LayerRegistry firstRegistry;
    LayerRegistry secondRegistry;

    firstRegistry.declare("M1");

    const Layer* otherM1 = secondRegistry.declare("M1");

    EXPECT_THROW(firstRegistry.mapGDS(otherM1, 15, 0), std::invalid_argument);
}