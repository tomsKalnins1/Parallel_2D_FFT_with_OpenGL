#include "Shader.h"

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

//	cout << "NAME OF THE FILES  = " << pathToVert << " ,  " << pathToFrag << endl;

	string vertexShader	= getFileContent(pathToVert);
	string fragmentShader = getFileContent(pathToFrag);

	//cout << pathToVert << "\n" << vertexShader << endl;
	//cout << pathToFrag << "\n" << fragmentShader << endl;

	const char* vertexShaderSource = vertexShader.c_str();
	const char* fragmentShaderSource = fragmentShader.c_str();

	//!!!!!!! TURNS OUT NVIDIA HAS SOME SORT OF A BUG WHERE THE DRIVER IS LIKELY COMPILING THE SHADERS
	//! WITHOUT ME COMPILING THEM EXPLICITLY (https://stackoverflow.com/questions/24171290/is-glcompileshader-optional)
	//! But in order fo my program to run for other platforms I need to leave those lines in!!!

	unsigned int vertID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertID, 1, &vertexShaderSource, NULL);
	glCompileShader(vertID);

	unsigned int fragID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragID, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragID);

	ID = glCreateProgram();

	glAttachShader(ID, vertID);
	glAttachShader(ID, fragID);

	glLinkProgram(ID);
	//! mistake always add linking error checking (wrote one letter in uppercase where it should haev been lowercase
	//! the linking of program failed silently)
	GLint linkSuccess = 0;
	glGetProgramiv(ID, GL_LINK_STATUS, &linkSuccess);
	if (linkSuccess == GL_FALSE) {
		char infoLog[1024];
		glGetProgramInfoLog(ID, 1024, NULL, infoLog);
		std::cout << "Framebuffer shader PROGRAM LINK FAILED:\n" << infoLog << std::endl;
	}



	glDeleteShader(vertID);
	glDeleteShader(fragID);

}

void Shader::Use() {
	//cout << "shaderID = " << ID << endl;
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