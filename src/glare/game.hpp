#ifndef GLARE_GAME_HPP
#define GLARE_GAME_HPP

#include "system.hpp"
#include "entity.hpp"

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
		GLFWwindow* window;

		Subsystem renderer;
		Subsystem physics;

		Entity_container ents;
	};
}

#endif // !GLARE_GAME_HPP
