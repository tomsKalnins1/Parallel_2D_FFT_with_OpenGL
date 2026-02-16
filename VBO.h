#ifndef VBO_H
#define VBO_H


#include <glad/glad.h>
#include <iostream>

class VBO {

public:
	unsigned int ID;
	GLfloat* vert;
	VBO(GLfloat* vertices, GLsizeiptr size);

	void Bind();

	void Unbind();

	void Delete();

	void vertToStr();



};

#endif