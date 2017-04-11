#include "gtest/gtest.h"
#include "../glare/slot_map.hpp"

TEST(SlotMap, DefaultConstructor)
{
	Glare::Slot_map<int> sm;
	EXPECT_EQ(sm.size(), 0);
}

TEST(SlotMap, AddRemove)
{
	Glare::Slot_map<int> sm;
	auto p1 = sm.add(1);
	ASSERT_TRUE(sm.is_valid(p1));
	EXPECT_EQ(sm[p1], 1);
	EXPECT_EQ(sm.size(), 1);

	auto p2 = sm.add(2);
	ASSERT_TRUE(sm.is_valid(p2));
	EXPECT_EQ(sm[p2], 2);
	EXPECT_EQ(sm.size(), 2);

	sm.remove(p1);
	EXPECT_FALSE(sm.is_valid(p1));
	EXPECT_EQ(sm.size(), 1);
	ASSERT_TRUE(sm.is_valid(p2));
	EXPECT_EQ(sm[p2], 2);

	auto p3 = sm.add(3);
	ASSERT_TRUE(sm.is_valid(p3));
	EXPECT_EQ(sm[p3], 3);
	EXPECT_EQ(sm.size(), 2);

	ASSERT_TRUE(sm.is_valid(p2));
	EXPECT_EQ(sm[p2], 2);

	sm.remove(p3);

	EXPECT_FALSE(sm.is_valid(p3));

	ASSERT_TRUE(sm.is_valid(p2));
	EXPECT_EQ(sm[p2], 2);
}

TEST(SlotMap, BufferedAddRemove)
{
	Glare::Slot_map<int> sm;

	auto p2 = sm.buffered_add(2);
	auto p1 = sm.buffered_add(1);
	sm.buffered_remove(p1);

	EXPECT_EQ(sm.size(), 0);
	EXPECT_FALSE(sm.is_valid(p1));
	EXPECT_FALSE(sm.is_valid(p2));

	sm.clean_buffers();

	EXPECT_EQ(sm.size(), 1);
	EXPECT_FALSE(sm.is_valid(p1));
	ASSERT_TRUE(sm.is_valid(p2));
	EXPECT_EQ(sm[p2], 2);
}

TEST(SlotMap, MultipleRemove)
{
	Glare::Slot_map<int> sm;
	auto p = sm.add(42);
	EXPECT_EQ(sm.size(), 1);
	ASSERT_TRUE(sm.is_valid(p));
	EXPECT_EQ(sm[p], 42);

	for (int i = 0; i < 3; ++i) {
		sm.remove(p);
		EXPECT_EQ(sm.size(), 0);
		EXPECT_FALSE(sm.is_valid(p));
	}
}

TEST(SlotMap, MultipleBufferedRemove)
{
	Glare::Slot_map<int> sm;
	auto p = sm.add(42);
	EXPECT_EQ(sm.size(), 1);
	ASSERT_TRUE(sm.is_valid(p));
	EXPECT_EQ(sm[p], 42);

	for (int i = 0; i < 3; ++i) {
		sm.buffered_remove(p);
		EXPECT_EQ(sm.size(), 1);
		ASSERT_TRUE(sm.is_valid(p));
		EXPECT_EQ(sm[p], 42);
	}

	sm.clean_buffers();
	EXPECT_EQ(sm.size(), 0);
	EXPECT_FALSE(sm.is_valid(p));
}

TEST(SlotMap, Clear)
{
	Glare::Slot_map<int> sm;
	sm.add(1);
	sm.add(2);
	sm.add(3);

	sm.clear();
	EXPECT_EQ(sm.size(), 0);
}

TEST(SlotMap, InitializerList)
{
	Glare::Slot_map<int> sm {0, 1, 2, 3};

	EXPECT_EQ(sm[0], 0);
	EXPECT_EQ(sm[1], 1);
	EXPECT_EQ(sm[2], 2);
	EXPECT_EQ(sm[3], 3);
}

TEST(SlotMap, Iterator)
{
	Glare::Slot_map<int> sm {0, 1, 2, 3};

	{ // TODO: update when VC++ is standards compliant
		auto iter = sm.begin();
		int i {0};
		for (/*auto[iter, i] = {sm.begin(), 0}*/; iter != sm.end(); ++iter, ++i) {
			EXPECT_EQ(*iter, i);
		}
	}

	auto iter = sm.begin();
	*iter = 42; // sm[0]
	EXPECT_EQ(sm[0], 42);

	iter[2] = -2; // sm[2]
	EXPECT_EQ(sm[2], -2);

	iter += 2; // sm[2]
	EXPECT_EQ(*iter, -2); // sm[2]
	EXPECT_EQ(iter[-2], 42); // sm[0]

	iter -= 1; // sm[1]
	EXPECT_EQ(*iter, 1);

	EXPECT_EQ(*(iter + 2), 3); // sm[3]
	EXPECT_EQ(*(iter - 1), 42); // sm[0]

	EXPECT_EQ(sm.end() - sm.begin(), sm.size());
}

TEST(SlotMap, ArrowOperator)
{
	Glare::Slot_map<std::pair<int, double>> sm;

	auto p1 = sm.add({1,2.0});
	ASSERT_TRUE(sm.is_valid(p1));
	EXPECT_EQ(sm[p1].first, 1);
	EXPECT_EQ(sm[p1].second, 2.0);

	auto iter = sm.begin();
	EXPECT_EQ(iter->first, 1);
	EXPECT_EQ(iter->second, 2.0);

	sm.add({3,4.0});
	sm.add({5,6.0});

	EXPECT_EQ(sm[1].first, 3);

	EXPECT_EQ(sm[2], (std::pair<int,double>{5,6.0}));
}

TEST(SlotMap, AddDefaultConstructor)
{
	Glare::Slot_map<int> sm;

	auto p1 = sm.add();
	auto p2 = sm.buffered_add();

	sm.clean_buffers();

	EXPECT_EQ(sm[p1], 0);
	EXPECT_EQ(sm[p2], 0);
}

TEST(SlotMap, IteratorConversions)
{
	using Iterator = typename Glare::Slot_map<int>::iterator;
	using Const_iterator = typename Glare::Slot_map<int>::const_iterator;

	Glare::Slot_map<int> sm;
	sm.add(42);

	Const_iterator cp1 {sm.cbegin()};
	Const_iterator cp2 {cp1};

	EXPECT_EQ(cp1, cp2);
	EXPECT_EQ(*cp1, 42);

	Iterator p1 {sm.begin()};
	Iterator p2 {p1};

	EXPECT_EQ(p1, p2);
	EXPECT_EQ(cp1, p1);

	Const_iterator p3 {p1};

	EXPECT_EQ(p1, p3);
}

TEST(SlotMap, PointerConversions)
{
	using Pointer = typename Glare::Slot_map<int>::pointer;
	using Const_pointer = typename Glare::Slot_map<int>::const_pointer;

	Glare::Slot_map<int> sm;
	Pointer p1 {sm.add(42)};

	ASSERT_TRUE(sm.is_valid(p1));

	EXPECT_EQ(p1, sm.begin());
	EXPECT_EQ(p1, sm.cbegin());
	EXPECT_EQ(sm[p1], 42);

	Pointer p2 {p1};
	ASSERT_TRUE(sm.is_valid(p2));
	EXPECT_EQ(p1, p2);

	Const_pointer p3 {p1};
	ASSERT_TRUE(sm.is_valid(p3));
	EXPECT_EQ(p3, sm.begin());
	EXPECT_EQ(p3, sm.cbegin());
	EXPECT_EQ(p1, p3);
}

TEST(SlotMap, IteratorToPointerConversions)
{
	using Iterator = typename Glare::Slot_map<int>::iterator;
	using Pointer = typename Glare::Slot_map<int>::pointer;
	using Const_pointer = typename Glare::Slot_map<int>::const_pointer;

	Glare::Slot_map<int> sm;
	sm.add(42);

	Iterator p1 {sm.begin()};
	EXPECT_EQ(*p1, 42);

	Pointer p2 {p1};
	ASSERT_TRUE(sm.is_valid(p2));
	EXPECT_EQ(p1, p2);
	EXPECT_EQ(sm[p2], 42);

	Const_pointer p3 {p1};
	ASSERT_TRUE(sm.is_valid(p3));
	EXPECT_EQ(p1, p3);
	EXPECT_EQ(sm[p3], 42);
}
