#include "Shader_source.h"
#include "ShaderProgram.h"



string getFileContent(const char* path) {

	ifstream in(path, ios::binary);

	if (in) {
	
		string content;
	
		in.seekg(0, ios::end);

		content.resize(in.tellg());


		in.seekg(0, ios::beg);

		in.read(&content[0], content.size());

		in.close();

	//	cout << content << '\n';

		return content;
	
	}

	throw(errno);

}

Shader::Shader(const char* pathToVert, const char* pathToFrag) {

	string fragmentShader = getFileContent(pathToFrag);

	shader_source vert(pathToVert, GL_VERTEX_SHADER);
	shader_source frag(pathToFrag, GL_FRAGMENT_SHADER);


	ID = glCreateProgram();

	glAttachShader(ID, vert.ID);
	glAttachShader(ID, frag.ID);

	glLinkProgram(ID);
	//! mistake always add linking error checking (wrote one letter in uppercase where it should haev been lowercase
	//! the linking of program failed silently)
	GLint linkSuccess = 0;
	glGetProgramiv(ID, GL_LINK_STATUS, &linkSuccess);
	if (linkSuccess == GL_FALSE) {
		char infoLog[1024];
		glGetProgramInfoLog(ID, 1024, NULL, infoLog);
		std::cout << "PROGRAM LINK FAILED:\n" << infoLog << std::endl;
	}

	glDeleteShader(vert.ID);
	glDeleteShader(frag.ID);

}

Shader::Shader(const char* compute_shader, fft_orientation orientation, int num_samples, int samples_per_processor) {

	shader_source comp(compute_shader, GL_COMPUTE_SHADER, orientation, num_samples, samples_per_processor);



	ID = glCreateProgram();

	glAttachShader(ID, comp.ID);


	glLinkProgram(ID);

	GLint linkSuccess = 0;
	glGetProgramiv(ID, GL_LINK_STATUS, &linkSuccess);
	if (linkSuccess == GL_FALSE) {
		char infoLog[1024];
		glGetProgramInfoLog(ID, 1024, NULL, infoLog);
		std::cout << "PROGRAM LINK FAILED:\n" << infoLog << std::endl;
	}

	glDeleteShader(comp.ID);

}

void Shader::Use() {
	glUseProgram(ID);
}

void Shader::Delete() {

	glDeleteProgram(ID);

}

void Shader::bindUniformTexToTexUnit(string uniformName, int texObjNr) {

	const char* name = uniformName.c_str();

	unsigned int location = glGetUniformLocation(ID, name);

	glUniform1i(location, texObjNr);

}

void Shader:: setUniform(unsigned int shader_id, string uniformName, glm::vec3 vector) {

	int progID;
	glGetIntegerv(GL_CURRENT_PROGRAM, &progID);

	if(shader_id != progID) {
	
		glUseProgram(shader_id);

	}


	unsigned int loc = glGetUniformLocation(shader_id, uniformName.c_str());

	glUniform3fv(loc, 1, &vector[0]);

}

void Shader::setUniform(unsigned int shader_id, string uniformName, glm::mat4 matrix) {

	int progID;
	glGetIntegerv(GL_CURRENT_PROGRAM, &progID);

	if (shader_id != progID) {

		glUseProgram(shader_id);

	}

	unsigned int loc = glGetUniformLocation(shader_id, uniformName.c_str());

	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));

}

void Shader::setUniform(unsigned int shader_id, string uniformName, float value) {

	int progID;
	glGetIntegerv(GL_CURRENT_PROGRAM, &progID);

	if (shader_id != progID) {

		glUseProgram(shader_id);

	}
	//const char* fuck = uniformName.c_str();

	unsigned int loc = glGetUniformLocation(shader_id, uniformName.c_str());

	glUniform1f(loc, value);

}


void Shader::setUniform(unsigned int shader_id, string uniformName,unsigned int value) {

	int progID;
	glGetIntegerv(GL_CURRENT_PROGRAM, &progID);

	if (shader_id != progID) {

		glUseProgram(shader_id);

	}
	//const char* fuck = uniformName.c_str();

	unsigned int loc = glGetUniformLocation(shader_id, uniformName.c_str());

	glUniform1i(loc, value);

}