#include "VBO.h"

using namespace std;

VBO::VBO(GLfloat* vertices, GLsizeiptr size) {
	vert = vertices;
	cout << "size of vertices = " << size << endl;
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

}

void VBO::Bind() {

	glBindBuffer(GL_ARRAY_BUFFER, ID);

}

void VBO::Unbind() {

	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void VBO::Delete() {

	glDeleteBuffers(1, &ID);

}

void VBO::vertToStr() {

	int s = sizeof(vert) / 4;

	for (int i = 0; i < s; ++i) {
	
		cout << vert[i] << endl;
	
	}

}