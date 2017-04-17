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

	em.component<TestA>(p1).s = "1 TestA";
	/*em.component<TestC>(p1).s = "1 TestC";

	em.component<TestA>(p2) = {"2 TestA"};
	em.component<TestB>(p2) = {"2 TestB"};

	em.component<TestB>(p3) = {"3 TestB"};
	em.component<TestC>(p3) = {"3 TestC"};

	em.component<TestA>(p4).s = "4 TestA";
	em.component<TestB>(p4).s = "4 TestB";
	em.component<TestC>(p4).s = "4 TestC";*/

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

//	for (auto x : em.filter()) {
//		TestA a = em.component<TestA>(p1); // throw if doesn't exist
//
//		if (em.has_component<TestB, TestC>(p3)) {
//			// auto [x, y] = em.component<B, C>(p3); // C++17 only
//
//			// TODO
//			// std::tuple<TestB, TestC> t = em.component<TestB, TestC>(p3);
//
//			TestB b = em.component<TestB>(p3);
//		}
//
//		{
//			// TODO
//			// std::tuple<TestB*, TestC*> t = em.check_component<TestB, TestC>(p3);
//			// TestB* b = std::get<TestB*>(x);
//
//			TestC* c = em.check_component<TestC>(p3);
//		}
//	}
//
//	for (auto x : em.filter<TestA>()) {
//
//	}
//
//	for (auto x : em.filter<TestA, TestB>()) {
//
//	}
//
//	for (auto x : em.filter<TestA, TestB, TestC>()) {
//
//	}
}
