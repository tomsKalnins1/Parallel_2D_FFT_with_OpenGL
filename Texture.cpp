#include "Texture.h"

Texture::Texture(GLenum internal_format, GLenum format, string file_name, int w, int h) :internal_color_format{ internal_format },
color_format{ format }, filename{ file_name }, width{ w }, height{ h } {

	glGenTextures(1, &ID);

	glBindTexture(GL_TEXTURE_2D, ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);




	

	if (filename != "no_file") {
		stbi_set_flip_vertically_on_load(true);
		image = stbi_loadf((file_name).c_str(), &width, &height, &numColorChannels, 0);

		glTexImage2D(GL_TEXTURE_2D, 0, internal_color_format, width, height, 0, color_format, GL_FLOAT, image);
		//glGenerateMipmap(GL_TEXTURE_2D);
		cout << "height = " << height << " width = " << width <<  '\n';
		cout << "file name  =  \n" << filename << '\n';
 
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, internal_color_format, width, height, 0, color_format, GL_FLOAT, NULL);
		cout << "height = " << height << " width = " << width << '\n';
		cout << "file name  =  \n" << filename << '\n';
	}

}

void Texture::bind_image_2D(int layout) {

	glBindImageTexture(layout, ID, 0, GL_FALSE, 0, GL_READ_WRITE, internal_color_format);

}

void Texture::activate_tex_unit(int unit) {

	glActiveTexture(GL_TEXTURE0 + unit);

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