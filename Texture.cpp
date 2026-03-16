#include "Texture.h"

Texture::Texture(string fileName = "no_file", GLenum internal_format = GL_RGBA32F, GLenum format = GL_RGBA, int w = 256, int h = 256) : filename{fileName}, 
internal_color_format{ internal_format }, color_format{ format }, width{ w }, height{ h } {



	glGenTextures(1, &ID);

	glBindTexture(GL_TEXTURE_2D, ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);



	stbi_set_flip_vertically_on_load(true);//images usualy have 0,0 at top left but opengl expects that coordinate at bottom left as for texture coordinates
	
	if (filename != "no_file") {

		image = stbi_loadf((fileName).c_str(), &width, &height, &numColorChannels, 0);

		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, image);
		glGenerateMipmap(GL_TEXTURE_2D);

	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 256, 0, GL_RGBA, GL_FLOAT, NULL);
	}



}

void Texture::ActivateTexUnit(int i) {

	if (i == 0) {
	
		glActiveTexture(GL_TEXTURE0);
	
	}

	if (i == 1) {

		glActiveTexture(GL_TEXTURE1);

	}

	if (i == 2) {

		glActiveTexture(GL_TEXTURE2);

	}
}

void Texture::Bind() {

	glBindTexture(GL_TEXTURE_2D, ID);

}

void Texture::Unbind() {

	glBindTexture(GL_TEXTURE_2D, 0);

}

void Texture::Delete() {

	glDeleteTextures(1, &ID);

}