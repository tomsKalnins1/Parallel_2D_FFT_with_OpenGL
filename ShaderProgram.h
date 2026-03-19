#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H


#include <glad/glad.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader_source.h"


using namespace std;


class Shader {

public:

	unsigned int ID;

	Shader(const char* vertex_shader, const char* fragment_shader);
	Shader(const char* compute_shader, fft_orientation orientation, int num_samples, int samples_per_processor);
	Shader() = delete;

	void Use();

	void Delete();

	void bindUniformTexToTexUnit(string uniformName, int texObjNr);

	static void setUniform(unsigned int shader_id, string uniformName, glm::vec3 vector);
	static void setUniform(unsigned int shader_id, string uniformName, glm::mat4 matrix);
	static void setUniform(unsigned int shader_id, string uniformName, float value);
	static void setUniform(unsigned int shader_id, string uniformName,unsigned int value);

	

};


#endif