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

float roundf(float var){

	float value = (int)(var * 100 + .5);
	return (float)value / 100;
}

extern string getFileContent(const char* path);

//vector<complex<float>> fftOutput(256);
float last_time = 0;
float delta_time = 0;


void saveImg(string path) {

	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * 512;
	stride += (stride % 4) ? (4 - stride % 4) : 0; //make sure the stride is a multiple of 4
	GLsizei bufferSize = stride * 512;
	vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, 512, 512, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
//	cout << "SIZE OF THE IMAGE VECTOR (FOR SAVED IMAGE) = " << buffer.size() << '\n';
	stbi_flip_vertically_on_write(true);
	stbi_write_png(path.c_str(), 512, 512, 3, buffer.data(), stride);

}

bool save = true;

struct Data {

	unsigned int bits;
	int forward;
	float norm;

};

int main() {


	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	

	GLFWwindow* window = glfwCreateWindow(1024, 1024, "Parallel 2D FFT", NULL, NULL);

	if (window == NULL) {

		glfwTerminate();

	}



	float plane[] = {

		// coords    // texCoords
		 0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.0f, 1.0f,

		 0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.0f, 1.0f

	};

	float plane_img[] = {

		// coords    // texCoords
		 0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.0f, 1.0f,

		 0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.0f, 1.0f

	};

	float plane_parts[] = {

		// coords    // texCoords
		 0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.0f, 1.0f,

		 0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.0f, 1.0f

	};

	glfwMakeContextCurrent(window);
//	glfwSetCursorPosCallback(window, mousePositionCallBack);
//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	//gladLoadGL();

	string v = "vert.vs";
	string f = "frag.fs";
	
	string source_vert = "source_img_vert.vs";
	string source_frag = "source_img_frag.fs";

	string parts_frag = "freq_comp_frag.fs";
	
	glViewport(0, 0, 1024, 1024);

	cout << "OpenGL version: " << glGetString(GL_VERSION) << '\n';

	Shader sh(v.c_str(), f.c_str());

	VAO vao;
	vao.Bind();
	VBO vbo(plane, sizeof(plane));
	vao.linkVBO(vbo, 2, 2);
	vao.Unbind();
	vbo.Unbind();

	Shader shF(source_vert.c_str(), source_frag.c_str());

	VAO vao_s;
	vao_s.Bind();
	VBO vbo_s(plane_img, sizeof(plane_img));
	vao_s.linkVBO(vbo_s, 2, 2);
	vao_s.Unbind();
	vbo_s.Unbind();

	Shader sh_parts(v.c_str(), parts_frag.c_str());

	VAO vao_p;
	vao_p.Bind();
	VBO vbo_p(plane_parts, sizeof(plane_parts));
	vao_p.linkVBO(vbo_p, 2, 2);
	vao_p.Unbind();
	vbo_p.Unbind();


	//----------------------------------------------------------------------------COMPUTE SHADER STUFF

	Shader compute_prog_h("fft_compute_horizontal.cs", HORIZONTAL, 512, 4);

	float* image_fft;
	string pathToImage = "C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\camera_man_larger_3.png";

	cout << "GL__TEXTURE1 = " << GL_TEXTURE2 << '\n';

	Texture texture(GL_RGBA32F, GL_RGBA, pathToImage, 512, 512);
	
	Texture first_fft(GL_RGBA32F, GL_RGBA, "no_file", 512, 512);

	Texture output_1(GL_RGBA32F, GL_RGBA, "no_file", 512, 512);

	texture.Bind();

	texture.bind_image_2D(0);

	Texture::activate_tex_unit(1);

	
	first_fft.Bind();

	first_fft.bind_image_2D(1);

	unsigned int bits = Shader::num_bits((unsigned int)texture.height);
	Data d = { bits, -1, 1.0f };

	unsigned int buffer_comp_h;
	glCreateBuffers(1, &buffer_comp_h);
	glNamedBufferStorage(buffer_comp_h, sizeof(Data), &d, GL_DYNAMIC_STORAGE_BIT);	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer_comp_h);
	
	compute_prog_h.Use();

	float time_0 = glfwGetTime();

	glDispatchCompute((unsigned int)ceil(512 / 512), (unsigned int)ceil(512 / 1), 1);

	Shader compute_prog_v("fft_compute_vertical.cs", VERTICAL, 512, 4);

	first_fft.bind_image_2D(0);

	output_1.bind_image_2D(1);

	unsigned int buffer_comp_v;
	glCreateBuffers(1, &buffer_comp_v);
	glNamedBufferStorage(buffer_comp_v, sizeof(Data), &d, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer_comp_v);

	compute_prog_v.Use();

	glDispatchCompute((unsigned int)ceil(512 / 1), (unsigned int)ceil(512 / 512), 1);

	float time_1 = glfwGetTime();

	cout << "TIME TOOK TO PEFORM FOURIER TRANSFORM = " << (time_1 - time_0) << '\n';

//----------------------------------------------------------------------------COMPUTE SHADER STUFF

//-----------------------------------------------------------------------------
//---------------------------------------DO INVERSE 2D FFT WITH COMPUTE SHADERS 

	Texture inter(GL_RGBA32F, GL_RGBA, "no_file", 512, 512);
	Texture output_2(GL_RGBA32F, GL_RGBA, "no_file", 512, 512);

	Texture::activate_tex_unit(0);
	output_1.Bind();
	output_1.bind_image_2D(0);

	Texture::activate_tex_unit(1);
	inter.Bind();
	inter.bind_image_2D(1);

	Data d_inv = { bits, 1 , 512.0f * 512.0f };
	Data d_inv_inter = { bits, 1, 1.0f };


	unsigned int buffer_comp_h_i;
	glCreateBuffers(1, &buffer_comp_h_i);
	glNamedBufferStorage(buffer_comp_h_i, sizeof(Data), &d_inv_inter, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer_comp_h_i);

	compute_prog_h.Use();
	glDispatchCompute((unsigned int)ceil(512 / 512), (unsigned int)ceil(512 / 1), 1);

	Texture::activate_tex_unit(2);
	inter.Bind();
	inter.bind_image_2D(0);

	Texture::activate_tex_unit(2);
	output_2.Bind();
	output_2.bind_image_2D(1);
	unsigned int buffer_comp_v_i;
	glCreateBuffers(1, &buffer_comp_v_i);
	glNamedBufferStorage(buffer_comp_v_i, sizeof(Data), &d_inv, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer_comp_v_i);

	compute_prog_v.Use();
	glDispatchCompute((unsigned int)ceil(512 / 1), (unsigned int)ceil(512 / 512), 1);


	glm::mat4 trans = glm::mat4(1.0f);








	int add_waves_1 = 0;
	unsigned int loc_num_waves_1 = glGetUniformLocation(sh.ID, "wave_lim_1");


	int add_waves = 0;
	unsigned int loc_num_waves = glGetUniformLocation(sh.ID, "wave_lim");
	unsigned int loc_N = glGetUniformLocation(sh.ID, "N");

	glUniform1f(loc_N, 512.0f);

	
	
	

	

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		glClearColor(0.9f, 0.4f, 0.2f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT);

		
		


	
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		unsigned int loc_num_waves = glGetUniformLocation(sh.ID, "wave_lim");
		unsigned int loc_N = glGetUniformLocation(sh.ID, "N");
		
		sh.Use();
		
		glActiveTexture(GL_TEXTURE5);

		output_1.Bind();

		int location_tex = glGetUniformLocation(sh.ID, "real_imag");
		glUniform1i(location_tex, 5);

		trans = glm::mat4(1.0f);

		trans = glm::translate(trans, glm::vec3(-0.5f, 0.5f, 0.0f));

		trans = glm::scale(trans, glm::vec3(0.8f, 0.8f, 0.0f));

		unsigned int loc_move = glGetUniformLocation(sh.ID, "move");

		glUniformMatrix4fv(loc_move, 1, GL_FALSE, glm::value_ptr(trans));
		Shader::setUniform(sh.ID, "move", trans);


		glUniform1f(loc_N, 512.0f);
		glUniform1i(loc_num_waves, add_waves);

		vao.Bind();
	

		glDrawArrays(GL_TRIANGLES, 0, 6);

		shF.Use();
		glActiveTexture(GL_TEXTURE5);
		output_2.Bind();
		trans = glm::mat4(1.0f);

		trans = glm::translate(trans, glm::vec3(0.5f, 0.5f, 0.0f));

		trans = glm::scale(trans, glm::vec3(0.8f, 0.8f, 0.0f));


		Shader::setUniform(shF.ID, "move", trans);
		Shader::setUniform(shF.ID, "filterTexture",(unsigned int) 5);

		vao_s.Bind();
		
		glDrawArrays(GL_TRIANGLES, 0, 6);

		sh_parts.Use();
		glActiveTexture(GL_TEXTURE6);
		output_1.Bind();

		Shader::setUniform(sh_parts.ID, "real_imag", (unsigned int)6);

		trans = glm::mat4(1.0f);

		trans = glm::translate(trans, glm::vec3(0.0f, -0.5f, 0.0f));

		trans = glm::scale(trans, glm::vec3(0.9f, 0.9f, 0.0f));

		Shader::setUniform(sh_parts.ID, "move", trans);

		loc_num_waves = glGetUniformLocation(sh_parts.ID, "wave_lim");
		loc_N = glGetUniformLocation(sh_parts.ID, "N");
		
		glUniform1f(loc_N, 512.0f);
		glUniform1i(loc_num_waves, add_waves);

		vao_p.Bind();

		glDrawArrays(GL_TRIANGLES, 0, 6);

		if (save) {

				//saveImg("C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\OMIT_IMAG_TEST_FFT_OUTPUT_.png");
				//save = false;
		}
		
		float curr_time = glfwGetTime();
		delta_time += curr_time - last_time;
		last_time = curr_time;
		if (delta_time > 0.1f && add_waves <= 256) {
			delta_time = 0;
			add_waves += 1;
		}

		glfwSwapBuffers(window);
		
		

		
//		glUniform1i(loc_num_waves, 512);
//		glUniform1i(loc_num_waves_1, 512);


	
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


