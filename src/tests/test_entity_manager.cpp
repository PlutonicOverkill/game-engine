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
	em.component<TestC>(p1).s = "1 TestC";

	em.component<TestA>(p2) = {"2 TestA"};
	em.component<TestB>(p2) = {"2 TestB"};

	em.component<TestB>(p3) = {"3 TestB"};
	em.component<TestC>(p3) = {"3 TestC"};

	em.component<TestA>(p4).s = "4 TestA";
	em.component<TestB>(p4).s = "4 TestB";
	em.component<TestC>(p4).s = "4 TestC";

	/*
		   p1 p2 p3 p4
	Test1  x  x     x
	Test2     x  x  x
	Test3  x     x  x
	*/

	for (auto x : em.range()) {
		TestA a = em.check_component<A>(p1); // throw if doesn't exist

		TestB b = em.assert_component<B>(p2); // crash if doesn't exist (no checking)

		if (em.has_component<B, C>(p3)) {
			// auto [x, y] = em.component<B, C>(p3); // C++17 only

			std::tuple<B, C> t = em.component<B, C>(p3);

			B b = em.component<B>(p3);
		}

		{
			std::tuple<B*, C*> t = em.component_ptr<B, C>(p3);
			B* b = std::get<B*>(x);

			D* d = en.component_ptr<D>(p3);
		}
	}

	for (auto x : em.range<Test1>()) {

	}

	for (auto x : em.range<Test1, Test2>()) {

	}

	for (auto x : em.range<Test1, Test2, Test3>()) {

	}
}
