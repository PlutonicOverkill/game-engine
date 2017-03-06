#include "window.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "glad/glad.h" // ORDER DEPENDENCY
#include "GLFW/glfw3.h"

#include <utility>
#include <iostream>
#include <string>

#define GLM_FORCE_RADIANS 1

void APIENTRY gl_callback(GLenum,
						  GLenum,
						  GLuint,
						  GLenum,
						  GLsizei,
						  const GLchar*,
						  const void*);

Glare::Video::Window::Window(int width, int height, std::string title,
							 bool fullscreen, bool debug = false)
{
	glfwDefaultWindowHints(); // so hints set before can't interfere
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // we want OpenGL 4.5
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, debug);
	
	win = glfwCreateWindow(width,
						   height,
						   title.c_str(),
						   (fullscreen) ? glfwGetPrimaryMonitor() : nullptr,
						   nullptr);
	if(!win) throw Glare::Video::Window_construction_failed{};
	
	glfwMakeContextCurrent(win);
	gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

	if(debug) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_callback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
}

Glare::Video::Window::Window(Window&& w) noexcept
	:win{w.win}
{
	w.win = nullptr;
}

Glare::Video::Window& Glare::Video::Window::operator=(Window&& w) noexcept
{
	std::swap(win, w.win);
	return *this;
}

Glare::Video::Window::~Window()
{
	if(win) glfwDestroyWindow(win);
}

void attach_shader
(GLuint program, GLenum type, const char * code)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &code, NULL);
	glCompileShader(shader);
	glAttachShader(program, shader);
	glDeleteShader(shader);
}

int main()
{
	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	glTextureStorage2D(texture, 1, GL_RGBA8, 1024, 1024);

	// size too large to be allocated on stack
	GLuint* image_data = new GLuint[960 * 540];
	for(int i = 0; i != 960 * 540; i++) {
		image_data[i] = rand();
	}
	glTextureSubImage2D(texture, 0, 0, 0, 960, 540, GL_RGBA,
						GL_UNSIGNED_BYTE, image_data);
	delete[] image_data;


	GLfloat data[8] = {
		-1,-1, -1, 1,
		1,-1,  1, 1,
	};
	GLuint buffer;
	glCreateBuffers(1, &buffer);
	glNamedBufferStorage(buffer, sizeof(data), data, 0);

	int buffer_index = 0;
	GLuint array;
	glCreateVertexArrays(1, &array);
	glVertexArrayVertexBuffer(
		array, buffer_index, buffer, 0, sizeof(GLfloat) * 2);

	int position_attrib = 0;
	glEnableVertexArrayAttrib(array, position_attrib);
	glVertexArrayAttribFormat(
		array, position_attrib, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(
		array, position_attrib, buffer_index);

	GLuint program = glCreateProgram();
	glObjectLabel(GL_PROGRAM, program, -1, "TextureCopy");

	std::string vert_data{std::experimental::filesystem::path
	{"F:/glare/glare/src/shaders/main.vert"}};
	std::string frag_data{std::experimental::filesystem::path
	{"F:/glare/glare/src/shaders/main.frag"}};

	attach_shader(program, GL_VERTEX_SHADER, vert_data.c_str());

	attach_shader(program, GL_FRAGMENT_SHADER, frag_data.c_str());

	glLinkProgram(program);
	GLint result;
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if(result != GL_TRUE) {
		char msg[10240];
		glGetProgramInfoLog(program, 10240, NULL, msg);
		std::cerr << "Linking program failed:" << msg;
		return -1;
	}

	glUseProgram(program);
	glBindImageTexture(
		0, texture, 0, false, 0, GL_READ_ONLY, GL_RGBA8);
	glProgramUniform1i(program, 0, 0);
	glBindVertexArray(array);
}

void APIENTRY gl_callback(GLenum source,
						  GLenum type,
						  GLuint id,
						  GLenum severity,
						  GLsizei /*length*/,
						  const GLchar* message,
						  const void* /*userParam*/)
{
	std::string source_msg;
	std::string type_msg;
	std::string severity_msg;

	switch(source) {
	case GL_DEBUG_SOURCE_API: source_msg = "API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: source_msg = "Window system";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: source_msg = "Shader compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: source_msg = "Third party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION: source_msg = "Application";
		break;
	case GL_DEBUG_SOURCE_OTHER: source_msg = "Other";
		break;
	default: source_msg = "None";
	}

	switch(type) {
	case GL_DEBUG_TYPE_ERROR: type_msg = "Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_msg = "Deprecated behavior";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: type_msg = "Undefined behavior";
		break;
	case GL_DEBUG_TYPE_PORTABILITY: type_msg = "Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE: type_msg = "Performance";
		break;
	case GL_DEBUG_TYPE_MARKER: type_msg = "Marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP: type_msg = "Push group";
		break;
	case GL_DEBUG_TYPE_POP_GROUP: type_msg = "Pop group";
		break;
	case GL_DEBUG_TYPE_OTHER: type_msg = "Other";
		break;
	default: type_msg = "None";
	}

	switch(severity) {
	case GL_DEBUG_SEVERITY_HIGH: severity_msg = "High";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM: severity_msg = "Medium";
		break;
	case GL_DEBUG_SEVERITY_LOW: severity_msg = "Low";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: severity_msg = "Notification";
		break;
	default: severity_msg = "None";
	}

	std::cerr << "OpenGL Error:\n"
		<< "ID:\t" << id
		<< "Source:\t" << source_msg
		<< "Type:\t" << type_msg
		<< "Severity:\t" << severity_msg
		<< "Message:\n"
		<< message << '\n';
}
