#include "game.hpp"

#include "GLFW/glfw3.h"

#include <chrono>

void Glare::Game::update(double dt)
{
	physics.update(dt, ents);
}

void Glare::Game::render(double dt)
{
	renderer.update(dt, ents);
}

void Glare::Game::run()
{
	// adapted from http://gafferongames.com/game-physics/fix-your-timestep/

	// time the simulation has been running (physics time)
	auto t = std::chrono::nanoseconds::zero();

	constexpr std::chrono::milliseconds dt{16}; // constant physics update rate

	constexpr double timescale{1.0}; // multiplier to update time by

	auto current_time = std::chrono::steady_clock::now(); // realtime
	auto accumulator = std::chrono::nanoseconds::zero(); // "leftover" time used for interpolating

	// State previous; // previous game state - used for interpolating
	// State current; // current game state

	constexpr std::chrono::milliseconds max_time{250};

	while(!glfwWindowShouldClose(window)) {
		auto new_time = std::chrono::steady_clock::now();
		auto frame_time = new_time - current_time;
		if(frame_time > max_time) // if sim gets too far behind, get it back up to date
			frame_time = max_time;
		current_time = new_time;

		// we can't add a double and a long long so we need a cast
		accumulator += std::chrono::duration_cast<decltype(accumulator)>
			(frame_time * timescale); // amount of time to simulate

		while(accumulator >= dt) {
			// previousState = currentState;
			// integrate(currentState, t, dt);
			t += dt;
			accumulator -= dt;
		}

		const auto alpha = accumulator / dt;

		// State state = currentState * alpha +	previousState * (1.0 - alpha);

		// render(state);
	}
}
