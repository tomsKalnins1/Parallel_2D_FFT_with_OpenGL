#ifndef SHADER_H
#define SHADER_H


#include <glad/glad.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::string;


class shader_source {
	public:
		unsigned int ID;
		shader_source() = delete;
		shader_source(const shader_source&) = delete;
		shader_source operator=(const shader_source&) = delete;

		shader_source(string file_path, GLenum shader_type);

		string getFileContent(const char* path);

};

#endif