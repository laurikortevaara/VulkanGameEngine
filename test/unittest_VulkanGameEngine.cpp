#include <iostream>
#include "gtest/gtest.h"

#include "VulkanGameEngine.hpp"

// unittest_VulkanGameEngine.cpp : Defines the entry point for the Engine module.
#
TEST(VulkanGameEngine_Tests, lessThanTenTest)
{
	VulkanGameEngine engine;
    EXPECT_EQ(true, engine.lessThanTen(9));	
	EXPECT_EQ(false, engine.lessThanTen(10));
	EXPECT_EQ(false, engine.lessThanTen(11));
	EXPECT_EQ(false, engine.lessThanTen(100));
}
