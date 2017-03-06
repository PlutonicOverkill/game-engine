#ifndef GLARE_WINDOW_HPP
#define GLARE_WINDOW_HPP

#include "glad/glad.h" // ORDER DEPENDENCY
#include "GLFW/glfw3.h"

#include <string>
#include <exception>

namespace Glare {
	namespace Video {
		/*
			Technically, this is the OpenGL context, but GLFW
			combines the window and context together so they
			are one object here.
		*/
		class Window {
		public:
			Window(int width, int height, std::string title, bool fullscreen, bool debug = false);

			Window(Window&&) noexcept;
			Window& operator=(Window&&) noexcept;

			~Window();
		private:
			GLFWwindow* win;
		};

		class Window_construction_failed : public std::exception {};
	}
}



#endif // !GLARE_WINDOW_HPP
