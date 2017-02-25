#ifndef GLARE_OPENGL_IMPL_HPP
#define GLARE_OPENGL_IMPL_HPP

#include "opengl.hpp"

#include <exception>

namespace Glare
{
	namespace Renderer
	{
		namespace Gl
		{
			/*
			Use this wrapper when calling OpenGL functions
			If an error occurs, an exception will be thrown:
			no having to check the error code
			*/
			/*template<typename F, typename... Args)
			void call(F&& f, Args&&... args)
			{
				f(std::forward<Args>(args)...);
			}

			void set_exception(Error e);
			

			extern "C" void callback(GLenum source,
									 GLenum type,
									 GLuint id,
									 GLenum severity,
									 GLsizei length,
									 const GLchar* message,
									 const void* userParam);*/
		}
	}
}

#endif // !GLARE_OPENGL_IMPL_HPP
