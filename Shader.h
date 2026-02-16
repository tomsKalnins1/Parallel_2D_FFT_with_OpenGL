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

using namespace std;

class Shader {

public:
	unsigned int ID;
	Shader(const char* vertexShader, const char* fragmendShader);

	void Use();

	void Delete();

	void bindUniformTexToTexUnit(string uniformName, int texObjNr);

	static void setUniform(unsigned int shader_id, string uniformName, glm::vec3 vector);
	static void setUniform(unsigned int shader_id, string uniformName, glm::mat4 matrix);
	static void setUniform(unsigned int shader_id, string uniformName, float value);
	static void setUniform(unsigned int shader_id, string uniformName,unsigned int value);

	

};


#endif