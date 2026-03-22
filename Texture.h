#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include <glad/glad.h>
#include "stb/stb_image.h"
#include <iostream>
#include <string>

using namespace std;

class Texture{

public:
	unsigned int ID;
	int height;
	int width;
	int numColorChannels;
	GLenum color_format;
	GLenum internal_color_format;
	float* image;
	string filename;

	Texture() = delete;
	Texture operator=(const Texture&) = delete;
	Texture(const Texture&) = delete;

	Texture(GLenum internal_format, GLenum format, string file_name = "no_file", int width = 256, int height = 256);

	void Bind();

	void bind_image_2D(int layout);

	void Unbind();

	void Delete();

	static void activate_tex_unit(int i);

};

#endif