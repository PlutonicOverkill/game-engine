#include "gtest/gtest.h"
#include "../glare/ecs.hpp"

#include <string>
#include <tuple>

struct TestA { std::string s; };
struct TestB { std::string s; };
struct TestC { std::string s; };

TEST(EntityManager, Constructor)
{
	Glare::Ecs::Entity_manager<TestA, TestB, TestC> em;

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
	}

	for (auto x : em.filter<TestA>()) { // loop over just TestAs

	}

	for (auto x : em.filter<TestA, TestB>()) { // loop over TestAs and TestBs

	}
}
