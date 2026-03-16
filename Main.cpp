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

#include <string>
#include <fstream>
#include <sstream>
#include <cerrno>

#include "Shader.h"
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

	string comp_file = "test_compute_horiz.cs";


	string comp_shader_text = getFileContent(comp_file.c_str());
	const char* comp_shader_source = comp_shader_text.c_str();

	GLuint comp_shader_id = glCreateShader(GL_COMPUTE_SHADER);
	
	glShaderSource(comp_shader_id, 1, &comp_shader_source, NULL);
	glCompileShader(comp_shader_id);

	GLuint comp_sh_program_id = glCreateProgram();
	glAttachShader(comp_sh_program_id, comp_shader_id);
	glLinkProgram(comp_sh_program_id);

	GLint linkSuccess = 0;
	glGetProgramiv(comp_sh_program_id, GL_LINK_STATUS, &linkSuccess);
	if (linkSuccess == GL_FALSE) {
		char infoLog[1024];
		glGetProgramInfoLog(comp_sh_program_id, 1024, NULL, infoLog);
		std::cout << "COMPUTE SAHDER PROGRAM LINK FAILED:\n" << infoLog << std::endl;
	}

	GLint success;
	glGetShaderiv(comp_shader_id, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[1024];
		glGetShaderInfoLog(comp_shader_id, 1024, NULL, infoLog);
		std::cout << "COMPUTE SHADER COMPILATION FAILED:\n" << infoLog << std::endl;
	}

	string comp_frag = "comp_frag.fs";
	Shader comp(v.c_str(), comp_frag.c_str());
	comp.Use();
	Shader::setUniform(comp.ID, "screen", (unsigned int)0);



	unsigned int texture;

	float* image_fft;
	string pathToImage = "C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\camera_man.png";
	int fftHeight, fftWidth, fftNumChannels;
	image_fft = stbi_loadf(pathToImage.c_str(), &fftWidth, &fftHeight, &fftNumChannels, 0);

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 256, 256, 0, GL_RGB, GL_FLOAT, image_fft);

//------------------------------------Bind the source texture befor even the output texture is bound, but it can also be done
									//  at render loop, but most importantly has to be bound to the RIGHT texture unit before comp shader dispatch
	unsigned int tex_source;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	tex_source = glGetUniformLocation(comp_sh_program_id, "screen");
	glUniform1i(tex_source, 0);

	unsigned int perm_out_1;//image2D after 1st permutation

	glGenTextures(1, &perm_out_1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, perm_out_1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 256, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindImageTexture(1, perm_out_1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); //FIRST ARG SPECIFIES THE BINDING BASEN ON WHICH COMP SHADER KNOWS WHICH IMAGE TEXTURE TO SAMPLE FROM




	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, perm_out_1);

	//----------------------------------------------Texture  rg32f after horizontal fft

	unsigned int first_fft;//image2D after 1st permutation

	glGenTextures(1, &first_fft);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, first_fft);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 256, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindImageTexture(2, first_fft, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); //FIRST ARG SPECIFIES THE BINDING BASEN ON WHICH COMP SHADER KNOWS WHICH IMAGE TEXTURE TO SAMPLE FROM




	//glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, first_fft);


	unsigned int comp_sh_tex_loc;


	glUseProgram(comp_sh_program_id);



	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glDispatchCompute((unsigned int)ceil(256 / 256), (unsigned int)ceil(256 / 1), 1);


	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//-------------------------------------------------------COMPUTE SHADER NR 2
	string comp_file_2 = "test_compute_vert.cs";


	string comp_shader_text_2 = getFileContent(comp_file_2.c_str());
	const char* comp_shader_source_2 = comp_shader_text_2.c_str();

	GLuint comp_shader_id_2 = glCreateShader(GL_COMPUTE_SHADER);

	glShaderSource(comp_shader_id_2, 1, &comp_shader_source_2, NULL);
	glCompileShader(comp_shader_id_2);

	GLuint comp_sh_program_id_2 = glCreateProgram();
	glAttachShader(comp_sh_program_id_2, comp_shader_id_2);
	glLinkProgram(comp_sh_program_id_2);

	GLint linkSuccess_2 = 0;
	glGetProgramiv(comp_sh_program_id_2, GL_LINK_STATUS, &linkSuccess_2);
	if (linkSuccess_2 == GL_FALSE) {
		char infoLog_2[1024];
		glGetProgramInfoLog(comp_sh_program_id_2, 1024, NULL, infoLog_2);
		std::cout << "COMPUTE  2  SAHDER PROGRAM LINK FAILED:\n" << infoLog_2 << std::endl;
	}

	GLint success_2;
	glGetShaderiv(comp_shader_id_2, GL_COMPILE_STATUS, &success_2);
	if (!success_2) {
		char infoLog_3[1024];
		glGetShaderInfoLog(comp_shader_id_2, 1024, NULL, infoLog_3);
		std::cout << "COMPUTE  2  SHADER COMPILATION FAILED:\n" << infoLog_3 << std::endl;
	}


	unsigned int second_fft;//image2D after 1st permutation



	





	/*
	glGenTextures(1, &second_fft);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, second_fft);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 256, 0, GL_RGBA, GL_FLOAT, NULL);
	*/

	 //FIRST ARG SPECIFIES THE BINDING BASEN ON WHICH COMP SHADER KNOWS WHICH IMAGE TEXTURE TO SAMPLE FROM
	unsigned int output_1;//image2D after 1st permutation

	glGenTextures(1, &output_1);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, output_1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 256, 0, GL_RGBA, GL_FLOAT, NULL);


	glUseProgram(comp_sh_program_id_2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, first_fft);

	glBindImageTexture(0, first_fft, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glBindImageTexture(1, output_1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

//	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glDispatchCompute((unsigned int)ceil(256 / 1), (unsigned int)ceil(256 / 256), 1);


	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


	//-----------------COMPUTE SHADER

	VAO vao;

	vao.Bind();

	VBO vbo(plane, sizeof(plane));

	vao.linkVBO(vbo, 2, 2);

	vao.Unbind();
	vbo.Unbind();

//	Texture tex("image.png", false);


	
	/*

	fft2D fft(256.0f, 256.0f);
	
	vector<complex<float>> fftInputRow;
	
	cout << "SIZE OF IMAGE INPUT ROW = " << fftInputRow.size() << '\n';
	unsigned char* image_fft;
	string pathToImage = "C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\camera_man.png";
	int fftHeight, fftWidth, fftNumChannels;
	image_fft = stbi_load(pathToImage.c_str(), &fftWidth, &fftHeight, &fftNumChannels, 0);
	cout << "NUM COLOR CHANNELS  =  " << fftNumChannels << " WIDTH = " << fftWidth << " HEIGHT = " << fftHeight << '\n';

	
	vector<vector<complex<float>>> imageData_rows;

	int x1 = 0;
	int y1 = 0;


	int sizeImg = fftHeight;
	
	for (int y = 0; y < sizeImg; y++) {
	
		vector<complex<float>> one_row;

		for (int x = 0; x < sizeImg; x ++) {
		
			complex<float> c_img((float)image_fft[y1]/255.0f, 0.0f);
			
			y1 += 3;//INCORRECT STRIDE MIGHT BE PROBLEM 1#
			
			one_row.push_back(c_img);

		}

		imageData_rows.push_back(one_row);

	}


	
	vector<vector<complex<float>>> image_2D_freq_sprectrum = fft.fft_2D(imageData_rows);

	float* data_texture = new float[256 * 256 * 2];


	int index = 0;

	int size_fft_output = image_2D_freq_sprectrum.size();

	for (int x = 0; x < size_fft_output; x++) {
	
		for (int y = 0;y < size_fft_output; y++) {
		

			*(data_texture + index) = image_2D_freq_sprectrum[x][y].real();

			*(data_texture + index + 1) = image_2D_freq_sprectrum[x][y].imag();

			index += 2;
		}
	
	}
	

	unsigned int fft_output_texture;
	glGenTextures(1, &fft_output_texture);


	glBindTexture(GL_TEXTURE_2D, fft_output_texture);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 3);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, 256, 256, 0, GL_RG, GL_FLOAT, data_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	*/


	
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


