#include "../glare/glare.hpp"

#include <iostream>
#include <tuple>
#include <string>
#include <cassert>

struct Logic_A {
	int a;
};
struct Logic_B {
	int b;
};
struct Logic_C {
	int c;
};

struct System_A {};
struct System_B {};
struct System_C {};

struct Render {};

struct Name {
	std::string name;
};

int main()
{
	using Components = std::tuple<Logic_A, Logic_B, Logic_C>;
	using Logic_systems = std::tuple<System_A, System_B, System_C>;
	using Render_systems = std::tuple<Render>;
	
	Glare::Ecs::Ecs<Logic_systems, Render_systems, Components> engine{};

	auto e1 = engine.add_entity();
	auto e2 = engine.add_entity();
	auto e3 = engine.add_entity();

	assert(engine.size() == 3);

	*(e1->component<Logic_A>()) = {1};
	*(e1->component<Logic_C>()) = {1};

	*(e3->component<Logic_B>()) = {3};

	auto e4 = engine.add_entity();

	*(e4->component<Logic_A>()) = {4};
	*(e4->component<Logic_B>()) = {4};
	*(e4->component<Logic_C>()) = {4};

	assert(engine.size() == 4);
	assert(e4 == engine[4]);

	// sequence is a sequence of entities
	auto sequence = engine.sequence<Logic_A, Logic_C>();

	// *i is an Entity<Logic_A, Logic_C>
	for(auto i = sequence.begin(); i != sequence.end(); ++i) {
		// if i points to an entity with Logic_A and Logic_C
		if(!i) continue;

		std::cout << *(i->component<Logic_A>())
			<< *(i->component<Logic_C>());
	}

	e2->remove();

	assert(e3 == engine[3]);

	engine.remove_entity(e3);
	engine.remove_entity[1];

	return 0;
}
