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

	Texture(string fileName, GLenum internal_format, GLenum format, int w, int h);

	void Bind();

	void BindUnit(int num_image_unit);

	void Unbind();

	void Delete();

	void ActivateTexUnit(int i);

};

#endif