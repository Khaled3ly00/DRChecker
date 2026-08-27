#include <gtest/gtest.h>

#include "drcheck/domain/Layer.h"

using drcheck::domain::Layer;
using drcheck::domain::layerFromString;

TEST(LayerTest, ParsesLayerStringToLayer)
{
	EXPECT_EQ(layerFromString("M1"), Layer::M1);
	EXPECT_EQ(layerFromString("M2"), Layer::M2);
	EXPECT_EQ(layerFromString("VIA1"), Layer::VIA1);
	EXPECT_EQ(layerFromString("PO"), Layer::PO);
	EXPECT_EQ(layerFromString("OD"), Layer::OD);
}