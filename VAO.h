#ifndef VAO_H
#define VAO_H

#include "VBO.h"
#include <iostream>


	extern class VAO {

	public:
		unsigned int ID;
		VAO();

		void linkVBO(VBO vbo, int numCoords, int numTexCoords);

		void Bind();

		void Unbind();

		void Delete();





	};



#endif