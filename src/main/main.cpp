#include <iostream>

#include "window.hpp"

int main(int argc, char* argv[])
{
	if(!Engine::Video::video_init())
		return 1;

	{
		Engine::Video::Window win{"OpenGL Window"};
	}

	Engine::Video::video_shutdown();

	return 0;
}
