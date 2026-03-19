#include <stdio.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include <cmath>
#include <math.h>

#include "ShaderProgram.h"
#include <string>
#include <fstream>
#include <sstream>
#include <cerrno>

#include "VAO.h"
#include "VBO.h"
#include "Texture.h"
#include "fft_2D_images.h"

#define THE_PI 3.1415926535897932385

using namespace std;

extern int extr;

extern string getFileContent(const char* path);

//vector<complex<float>> fftOutput(256);
float last_time = 0;
float delta_time = 0;


void saveImg(string path) {

	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * 256;
	stride += (stride % 4) ? (4 - stride % 4) : 0; //make sure the stride is a multiple of 4
	GLsizei bufferSize = stride * 256;
	vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
//	cout << "SIZE OF THE IMAGE VECTOR (FOR SAVED IMAGE) = " << buffer.size() << '\n';
	stbi_flip_vertically_on_write(true);
	stbi_write_png(path.c_str(), 256, 256, 3, buffer.data(), stride);

}

bool save = true;

int main() {


	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	

	GLFWwindow* window = glfwCreateWindow(256, 256, "Framebuffer", NULL, NULL);

	if (window == NULL) {

		glfwTerminate();

	}



	float plane[] = {

		// coords    // texCoords
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f,

		 1.0f,  1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f

	};

	glfwMakeContextCurrent(window);
//	glfwSetCursorPosCallback(window, mousePositionCallBack);
//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	gladLoadGL();

	string v = "vert.vs";
	string f = "frag.fs";
	
	string frame_b_vert = "vertFrameBuffer.vs";
	string frabe_b_frag = "fragFrameBuffer.fs";
	
	glViewport(0, 0, 256, 256);

	cout << "OpenGL version: " << glGetString(GL_VERSION) << '\n';

	Shader sh(v.c_str(), f.c_str());

	Shader shF(frame_b_vert.c_str(), frabe_b_frag.c_str());


	//-----------------COMPUTE SHADER




	Shader compute_prog_h("fft_compute_horizontal.cs", HORIZONTAL, 256, 4);


	//-----------------COMPUTE SHADER
	//-----------------TEXTURING
	unsigned int texture;

	float* image_fft;
	string pathToImage = "C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\camera_man.png";
	int fftHeight, fftWidth, fftNumChannels;
	image_fft = stbi_loadf(pathToImage.c_str(), &fftWidth, &fftHeight, &fftNumChannels, 0);

	glGenTextures(1, &texture);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 256, 256, 0, GL_RGB, GL_FLOAT, image_fft);


	//----------------------------------------------Texture  rg32f after horizontal fft

	unsigned int first_fft;//image2D after 1st permutation

	glGenTextures(1, &first_fft);
	/*Texure has an ID and it can be bound to a certain texture unit,apparently when binding it and to */
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, first_fft);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 256, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindImageTexture(2, first_fft, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); //FIRST ARG SPECIFIES THE BINDING BASEN ON WHICH COMP SHADER KNOWS WHICH IMAGE TEXTURE TO SAMPLE FROM



	
	compute_prog_h.Use();


	//glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glDispatchCompute((unsigned int)ceil(256 / 256), (unsigned int)ceil(256 / 1), 1);


	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//-------------------------------------------------------COMPUTE SHADER NR 2


	Shader compute_prog_v("fft_compute_vertical.cs", VERTICAL, 256, 4);



	unsigned int second_fft;//image2D after 1st permutation




	unsigned int output_1;//image2D after 1st permutation

	glGenTextures(1, &output_1);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, output_1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 256, 0, GL_RGBA, GL_FLOAT, NULL);


	compute_prog_v.Use();



	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, first_fft);

	glBindImageTexture(0, first_fft, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glBindImageTexture(1, output_1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);



	glDispatchCompute((unsigned int)ceil(256 / 1), (unsigned int)ceil(256 / 256), 1);


	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


	//-----------------COMPUTE SHADER

	VAO vao;

	vao.Bind();

	VBO vbo(plane, sizeof(plane));

	vao.linkVBO(vbo, 2, 2);

	vao.Unbind();
	vbo.Unbind();



	
	int add_waves = 0;
	unsigned int loc_num_waves = glGetUniformLocation(sh.ID, "wave_lim");

	


	

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		glClearColor(0.9f, 0.4f, 0.2f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT);

		
		


	
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		/*
		comp.Use();

		unsigned int tex_loc_cs;
		tex_loc_cs = glGetUniformLocation(comp.ID, "screen");
		glUniform1i(tex_loc_cs, 3);//mistake before due to forgettig to set this to 1 which was 0 that was set earlier
//		Shader::setUniform(sh.ID, "screen", (unsigned int)0);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, output_1);

		*/
		
		
		
		sh.Use();
		
		glActiveTexture(GL_TEXTURE3);
		//glBindTexture(GL_TEXTURE_2D, texture);
		glBindTexture(GL_TEXTURE_2D, output_1);
		//glBindTexture(GL_TEXTURE_2D, first_fft);

		int location_tex = glGetUniformLocation(sh.ID, "real_imag");
		glUniform1i(location_tex, 3);
		glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
		
		

		vao.Bind();
	

		glDrawArrays(GL_TRIANGLES, 0, 6);

	


		if (save) {

				saveImg("C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\COMP_SHADER_TEST_8.png");
				//save = false;
		}
		
		

		glfwSwapBuffers(window);

		
		float curr_time = glfwGetTime();
		delta_time += curr_time - last_time;
		last_time = curr_time;
	//	cout << "DELTA TIME = " << delta_time;
		if (delta_time > 0.1f && add_waves <= 256) {
			delta_time = 0;
			add_waves+=1;

		//	cout << add_waves;
		}
		glUniform1i(loc_num_waves, add_waves);

		add_waves = (add_waves <= 128) ? add_waves += 1 : 128;
	
	}
	

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;

}

string getFileContent1(const char* path) {

	ifstream in(path, ios::binary);

	if (in) {

		string content;

		in.seekg(0, ios::end);

		content.resize(in.tellg());

		in.seekg(0, ios::beg);

		in.read(&content[0], content.size());

		in.close();



		return content;

	}

	throw(errno);

}


