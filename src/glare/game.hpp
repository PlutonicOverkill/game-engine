#ifndef GLARE_GAME_HPP
#define GLARE_GAME_HPP

#include "system.hpp"
#include "entity.hpp"
#include "window.hpp"

#include <vector>
#include <memory>

namespace Glare {
/*
	Game class, manages all subsystems and stuff
*/
	class Game {
	public:
		void update(double dt);

		void render(double dt);

		void run();

	private:
		Video::Window win;

		Subsystem renderer;
		Subsystem physics;

		Factory<Component> component_maker;

		Entity::Entity_container ents;
	};
}

#endif // !GLARE_GAME_HPP
