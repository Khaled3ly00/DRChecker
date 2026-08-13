#include <gtest/gtest.h>

#include "drcheck/domain/Layer.h"

using drcheck::domain::Layer;
using drcheck::domain::layerFromString;

TEST(LayerTest, ParsesLayerStringToLayer)
{
	EXPECT_EQ(layerFromString("Metal1"), Layer::Metal1);
	EXPECT_EQ(layerFromString("Metal2"), Layer::Metal2);
	EXPECT_EQ(layerFromString("Via12"), Layer::Via12);
	EXPECT_EQ(layerFromString("Poly"), Layer::Poly);
	EXPECT_EQ(layerFromString("Diffusion"), Layer::Diffusion);
}