#include "window.hpp"

Engine::Video::Window::Window(std::string title, int width, int height)
	:id{glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr)}
{
	if(!id)
		throw Engine::Video::Window_creation_failed{"GLFW window creation failed."};
}

Engine::Video::Window::~Window()
{
	glfwDestroyWindow(id);
}

bool Engine::Video::video_init()
{
	return glfwInit() == GLFW_TRUE;
}

void Engine::Video::video_shutdown()
{
	glfwTerminate();
}
