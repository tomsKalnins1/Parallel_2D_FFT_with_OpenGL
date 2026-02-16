#include "VAO.h"

using namespace std;



VAO::VAO() {

	glGenVertexArrays(1, &ID);

}

void VAO::linkVBO(VBO vbo, int numCoords, int numTexCoords) {

	vbo.Bind();

	glBindVertexArray(ID);

	if (numCoords == 2) {
	
		if (numTexCoords == 2)

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
			//accidentally typing 2 instead of 1 in the first arg cause access vialition for glDrawArrays

			for (int i = 0; i < 6; ++i) {
			
				cout << "vbo coor = " << vbo.vert[i] << endl;

			}
	}

	if (numCoords == 3) {
	
		if (numTexCoords == 2) {
			cout << "cumCoords = 3, nuTexCoords = 2" << endl;
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glEnableVertexAttribArray(2);


		
		}

		if (numCoords == 0) {
		
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
		
		}


	
	}
}

void VAO::Bind() {

	glBindVertexArray(ID);

	

}

void VAO::Unbind() {

	glBindVertexArray(0);

}

void VAO::Delete() {

	glDeleteVertexArrays(1, &ID);

}

