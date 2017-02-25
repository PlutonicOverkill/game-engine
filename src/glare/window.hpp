#ifndef GLARE_OPENGL_HPP
#define GLARE_OPENGL_HPP



#include <iostream>
#include <string>
#include <filesystem>

/*
window hints
create window
make context current
gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
*/



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
	{"F:\\glare\\glare\\src\\shaders\\main.vert"}};
	std::string frag_data{std::experimental::filesystem::path
	{"F:\\glare\\glare\\src\\shaders\\main.frag"}};

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

#endif // !GLARE_OPENGL_HPP
