#include "gtest/gtest.h"
#include "../glare/slot_map.hpp"

TEST(SlotMap, Test)
{
	// throw exceptions when pointers, etc. are not valid
	
	Glare::Slot_map<int> sm;

	EXPECT_EQ(sm.size(), 0);

	auto p1 = sm.add(1);
	EXPECT_TRUE(p1);
	EXPECT_EQ(*p1, 1);
	EXPECT_EQ(sm.size(), 1);
	auto p2 = sm.add(2);
	EXPECT_TRUE(p2);
	EXPECT_EQ(*p2, 2);
	EXPECT_EQ(sm.size(), 2);

	auto p3 = sm.buffered_add(3);
	auto p4 = sm.buffered_add(4);
	p2.buffered_remove();
	EXPECT_FALSE(p2);
	EXPECT_EQ(sm.size(), 2);

	p4.remove();
	EXPECT_FALSE(p4);
	EXPECT_EQ(sm.size(), 2);

	sm.clean_buffers();
	EXPECT_EQ(sm.size(), 3);

	auto p5 = sm.buffered_add(5);
	p5.buffered_remove();
	sm.clean_buffers();
	EXPECT_EQ(sm.size(), 3);

	sm.clear();
	EXPECT_EQ(sm.size(), 0);
	
	// clear
	// Slot_map()
	// Slot_map(std::initializer_list<T>)
	// operator=(std::initializer_list<T>)

	//Pointer:
	// constructor
	// default constructor
	// remove
	// reset
	// bool
	// !=
	// * (const)
	// ==
	// -> (const)

	//Iterator:
	// buffered_remove
	// constructor
	// remove
	// -
	// --
	// !=
	// * (const)
	// [] (const)
	// ++
	// +=
	// -=
	// ==
	// -> (const)
}
