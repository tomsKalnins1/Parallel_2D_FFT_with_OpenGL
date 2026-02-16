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
	unsigned int fboID;
	int height;
	int width;
	int numColorChannels;
	unsigned char* image;

	Texture(string fileName, bool useFrameBuffer);

	void Bind();

	void Unbind();

	void Delete();

	void ActivateTexUnit(int i);

};

#endif