#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <string>

#include <GLFW/glfw3.h>

#include "error.hpp"

namespace Engine
{
	namespace Video
	{
		bool video_init();
		void video_shutdown();
		
		class Window_creation_failed : public Engine_error {
			using Engine_error::Engine_error; // inherit constructors
		};

		class Window {
		public:
			Window(std::string title, int width = default_width, int height = default_height);
			~Window();
		private:
			GLFWwindow* id;

			static constexpr int default_width{640};
			static constexpr int default_height{480};
		};
	}
}

#endif // WINDOW_HPP
