#include "gtest/gtest.h"
#include "../glare/slot_map.hpp"

TEST(LogicTest, TestValues)
{
	// EXPECT_EQ
	// ASSERT_EQ

	// EXPECT_TRUE
	// etc.

	EXPECT_EQ(42, 42) << "42 == 42";
	EXPECT_EQ(59, 33) << "59 == 33";
}

