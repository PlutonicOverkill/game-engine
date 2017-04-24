#include "gtest/gtest.h"
#include "../glare/ecs.hpp"

#include <string>
#include <tuple>

struct TestA { std::string s; };
struct TestB { std::string s; };
struct TestC { std::string s; };

TEST(EntityManager, Constructor)
{
	Glare::Entity_manager<TestA, TestB, TestC> em;

	auto p1 = em.add();
	auto p2 = em.add();
	auto p3 = em.add();
	auto p4 = em.add();

	em.make_component<TestA>(p1).s = "1 TestA";
	em.make_component<TestC>(p1).s = "1 TestC";

	em.make_component<TestA>(p2) = {"2 TestA"};
	em.make_component<TestB>(p2) = {"2 TestB"};

	em.make_component<TestB>(p3) = {"3 TestB"};
	em.make_component<TestC>(p3) = {"3 TestC"};

	em.make_component<TestA>(p4).s = "4 TestA";
	em.make_component<TestB>(p4).s = "4 TestB";
	em.make_component<TestC>(p4).s = "4 TestC";

	typename Glare::Entity_manager<TestA, TestB, TestC>::Range<> range = em.filter();
	typename Glare::Entity_manager<TestA, TestB, TestC>::Range_const<> crange = em.cfilter();

	typename Glare::Entity_manager<TestA, TestB, TestC>::Component_range<TestA> rangeA = em.filter<TestA>();
	typename Glare::Entity_manager<TestA, TestB, TestC>::Component_range_const<TestA> crangeA = em.cfilter<TestA>();

	Glare::Entity_manager<TestA, TestB, TestC>::Range<TestA, TestB> rangeAB = em.filter<TestA, TestB>();
	Glare::Entity_manager<TestA, TestB, TestC>::Range_const<TestA, TestB> crangeAB = em.cfilter<TestA, TestB>();

	/*
		   p1 p2 p3 p4
	TestA  x  x     x
	TestB     x  x  x
	TestC  x     x  x
	*/

	// component access functions:
	// make_component(): create it if it doesn't exist
	// component(): throw if it doesn't exist
	// check_component(): return nullptr if it doesn't exist

	// has_component(): return true if component exists

	for (auto x : em.filter()) { // loop over any components
		// auto p = x.check_component<TestA>();
		bool b = x.has_component<TestB>();
	}

	for (auto x : em.filter<TestA>()) { // loop over just TestAs
		auto p = x.component<TestA>();
		bool b = x.has_component<TestB>();
		auto q = x.check_component<TestA>();
		auto m = x.make_component<TestC>();
	}

	for (auto x : em.filter<TestA, TestB>()) { // loop over TestAs and TestBs
		// auto p = x.component<TestA>();
		// auto q = x.component<TestB>();
		// auto m = x.make_component<TestC>();
	}

	const Glare::Entity_manager<TestA, TestB, TestC> cem;

	for (auto x : cem.filter()) {}

	for (auto x : cem.filter<TestA>()) {}

	for (auto x : cem.filter<TestA, TestB>()) {}
}
