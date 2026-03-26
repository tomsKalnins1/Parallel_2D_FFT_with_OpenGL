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
	int vertical;
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

	float fft_texture_output[] = {

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

	//---------------------------------------------------------------------------------DISPLAY ANIAMTED IMAGE ASSEMBLEY

	Shader sh(v.c_str(), f.c_str());

	VAO vao;
	vao.Bind();
	VBO vbo(plane, sizeof(plane));
	vao.linkVBO(vbo, 2, 2);
	vao.Unbind();
	vbo.Unbind();

	//---------------------------------------------------------------------------------DISPLAY ORIGINAL IMAGE

	Shader shF(source_vert.c_str(), source_frag.c_str());

	VAO vao_s;
	vao_s.Bind();
	VBO vbo_s(plane_img, sizeof(plane_img));
	vao_s.linkVBO(vbo_s, 2, 2);
	vao_s.Unbind();
	vbo_s.Unbind();

	//---------------------------------------------------------------------------------DISPLAY INDIVIDUAL FREQUENCIES

	Shader sh_parts(v.c_str(), parts_frag.c_str());

	VAO vao_p;
	vao_p.Bind();
	VBO vbo_p(plane_parts, sizeof(plane_parts));
	vao_p.linkVBO(vbo_p, 2, 2);
	vao_p.Unbind();
	vbo_p.Unbind();

	//---------------------------------------------------------------------------------DISPLAY THE FFT OUTPUT

	VAO vao_out;
	vao_out.Bind();
	VBO vbo_out(fft_texture_output, sizeof(fft_texture_output));
	vao_out.linkVBO(vbo_out, 2, 2);
	vao_out.Unbind();
	vbo_out.Unbind();




	//----------------------------------------------------------------------------COMPUTE SHADER STUFF

	Shader compute_prog_h("fft_compute_horizontal.cs", HORIZONTAL, 256, 4);

	float* image_fft;
	string pathToImage = "C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\camera_man.png";

	cout << "GL__TEXTURE1 = " << GL_TEXTURE2 << '\n';

	Texture texture(GL_RGBA32F, GL_RGBA, pathToImage, 256, 256);
	
	Texture first_fft(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);

	Texture transp_FFT_1(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);

	Texture second_fft(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);
	
	Texture output_1(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);


	Texture::activate_tex_unit(0);
	texture.Bind();
	texture.bind_image_2D(0);

	Texture::activate_tex_unit(1);
	first_fft.Bind();
	first_fft.bind_image_2D(1);

	Texture::activate_tex_unit(2);
	transp_FFT_1.Bind();
	transp_FFT_1.bind_image_2D(2);

	unsigned int bits = Shader::num_bits((unsigned int)texture.height);
	Data d = { bits, -1, 0, 1.0f };
	Data d_vert = { bits, -1, 1, 1.0f };

	unsigned int buffer_comp_h;
	glCreateBuffers(1, &buffer_comp_h);
	glNamedBufferStorage(buffer_comp_h, sizeof(Data), &d, GL_DYNAMIC_STORAGE_BIT);	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, buffer_comp_h);
	
	

	compute_prog_h.Use();

	float time_0 = glfwGetTime();

	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256 / 1), 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	string path_trans_v = "transpose_CW.cs";

	Shader rot(path_trans_v.c_str(), HORIZONTAL, 256, 4);

	Texture transpose_T(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);


	Texture::activate_tex_unit(7);
	first_fft.Bind();
	first_fft.bind_image_2D(0);


	Texture::activate_tex_unit(8);
	transp_FFT_1.Bind();
	transp_FFT_1.bind_image_2D(1);


	rot.Use();
	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256), 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	Shader compute_prog_h_2("fft_compute_horizontal.cs", HORIZONTAL, 256, 4);

	Texture::activate_tex_unit(0);
	transp_FFT_1.Bind();
	transp_FFT_1.bind_image_2D(0);

	Texture::activate_tex_unit(1);
	second_fft.Bind();
	second_fft.bind_image_2D(1);

	Texture::activate_tex_unit(2);
	output_1.Bind();
	output_1.bind_image_2D(2);


	unsigned int buffer_comp_h_v;
	glCreateBuffers(1, &buffer_comp_h_v);
	glNamedBufferStorage(buffer_comp_h_v, sizeof(Data), &d_vert, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, buffer_comp_h_v);

	compute_prog_h_2.Use();

	time_0 = glfwGetTime();

	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256 / 1), 1);



	Shader rot_2(path_trans_v.c_str(), HORIZONTAL, 256, 4);

	Texture transpose_T_2(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);


	Texture::activate_tex_unit(7);
	second_fft.Bind();
	second_fft.bind_image_2D(0);


	Texture::activate_tex_unit(8);
	transpose_T_2.Bind();
	transpose_T_2.bind_image_2D(1);


	rot_2.Use();
	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256), 1);


//----------------------------------------------------------------------------COMPUTE SHADER STUFF

//-----------------------------------------------------------------------------
//---------------------------------------DO INVERSE 2D FFT WITH COMPUTE SHADERS 
	Shader compute_prog_h_3("fft_compute_horizontal.cs", HORIZONTAL, 256, 4);

	Texture fft_1(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);
	Texture fft_1_transp(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);
	Texture fft_2(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);
	Texture ifft(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);

	Texture::activate_tex_unit(0);
	transpose_T_2.Bind();
	transpose_T_2.bind_image_2D(0);

	Texture::activate_tex_unit(1);
	fft_1.Bind();
	fft_1.bind_image_2D(1);

	Data d_ifft = { bits, 1, 1, 1.0f };
	Data d_ifft_1 = { bits, 1, 1, 255.0f * 255.0f };

	unsigned int buffer_comp_h_3;
	glCreateBuffers(1, &buffer_comp_h_3);
	glNamedBufferStorage(buffer_comp_h_3, sizeof(Data), &d_ifft, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, buffer_comp_h_3);

	compute_prog_h_3.Use();

	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256 / 1), 1);

	//------------------------------------------------------TRANSPOSE 3

	Shader rot_3(path_trans_v.c_str(), HORIZONTAL, 256, 4);

	Texture transpose_T_3(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);


	Texture::activate_tex_unit(7);
	fft_1.Bind();
	fft_1.bind_image_2D(0);


	Texture::activate_tex_unit(8);
	transpose_T_3.Bind();
	transpose_T_3.bind_image_2D(1);


	rot_3.Use();
	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256), 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	//-------------------------------------------------------------IFFT COLUMNS
	Shader compute_prog_h_4("fft_compute_horizontal.cs", HORIZONTAL, 256, 4);

	Texture ifft_out(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);



	Texture::activate_tex_unit(0);
	transpose_T_3.Bind();
	transpose_T_3.bind_image_2D(0);

	Texture::activate_tex_unit(1);
	ifft_out.Bind();
	ifft_out.bind_image_2D(1);

	unsigned int buffer_comp_h_4;
	glCreateBuffers(1, &buffer_comp_h_4);
	glNamedBufferStorage(buffer_comp_h_4, sizeof(Data), &d_ifft_1, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, buffer_comp_h_4);

	compute_prog_h_3.Use();

	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256 / 1), 1);

	//------------------------------------------------------TRANSPOSE 3

	Shader rot_4(path_trans_v.c_str(), HORIZONTAL, 256, 4);

	Texture transpose_T_4(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);


	Texture::activate_tex_unit(7);
	ifft_out.Bind();
	ifft_out.bind_image_2D(0);


	Texture::activate_tex_unit(8);
	transpose_T_4.Bind();
	transpose_T_4.bind_image_2D(1);


	rot_4.Use();
	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256), 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//------------------------------------------------------TRANSPOSE 3

	glm::mat4 trans = glm::mat4(1.0f);

	int add_waves = 0;
	int add_waves_1 = 0;

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		glClearColor(0.156f, 0.427f, 0.482f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT);

		
		


	
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		unsigned int loc_num_waves_1 = glGetUniformLocation(sh.ID, "wave_lim_1");
		unsigned int loc_num_waves = glGetUniformLocation(sh.ID, "wave_lim");
		unsigned int loc_N = glGetUniformLocation(sh.ID, "N");
		glUniform1f(loc_N, 256.0f);
		
		sh.Use();
		
		glActiveTexture(GL_TEXTURE5);

		//animated image assembley

		transpose_T_2.Bind();

		int location_tex = glGetUniformLocation(sh.ID, "real_imag");
		glUniform1i(location_tex, 5);

		trans = glm::mat4(1.0f);

		trans = glm::translate(trans, glm::vec3(-0.5f, 0.5f, 0.0f));

		trans = glm::scale(trans, glm::vec3(0.8f, 0.8f, 0.0f));

		unsigned int loc_move = glGetUniformLocation(sh.ID, "move");

		glUniformMatrix4fv(loc_move, 1, GL_FALSE, glm::value_ptr(trans));
		Shader::setUniform(sh.ID, "move", trans);


		glUniform1f(loc_N, 256.0f);
		glUniform1i(loc_num_waves, add_waves);
		glUniform1i(loc_num_waves_1, add_waves_1);


		vao.Bind();
	

		glDrawArrays(GL_TRIANGLES, 0, 6);
		//original image but reassembled by compute shaders
		shF.Use();
		glActiveTexture(GL_TEXTURE5);
		transpose_T_4.Bind();
		trans = glm::mat4(1.0f);

		trans = glm::translate(trans, glm::vec3(0.5f, 0.5f, 0.0f));

		trans = glm::scale(trans, glm::vec3(0.8f, 0.8f, 0.0f));


		Shader::setUniform(shF.ID, "move", trans);
		Shader::setUniform(shF.ID, "filterTexture",(unsigned int) 5);

		vao_s.Bind();

		glDrawArrays(GL_TRIANGLES, 0, 6);

		//the fft output
		glActiveTexture(GL_TEXTURE6);
		second_fft.Bind();

		trans = glm::mat4(1.0f);

		trans = glm::translate(trans, glm::vec3(0.5f, -0.4f, 0.0f));

		trans = glm::scale(trans, glm::vec3(0.8f, 0.8f, 0.0f));


		Shader::setUniform(shF.ID, "move", trans);
		Shader::setUniform(shF.ID, "filterTexture", (unsigned int)6);

		vao_out.Bind();
		//individual freq.
		glDrawArrays(GL_TRIANGLES, 0, 6);

		sh_parts.Use();
		glActiveTexture(GL_TEXTURE7);
		output_1.Bind();

		Shader::setUniform(sh_parts.ID, "real_imag", (unsigned int)7);

		trans = glm::mat4(1.0f);

		trans = glm::translate(trans, glm::vec3(-0.5f, -0.4f, 0.0f));

		trans = glm::scale(trans, glm::vec3(0.8f, 0.8f, 0.0f));

		Shader::setUniform(sh_parts.ID, "move", trans);

		loc_num_waves = glGetUniformLocation(sh_parts.ID, "wave_lim");
		loc_num_waves_1 = glGetUniformLocation(sh_parts.ID, "wave_lim_1");
		loc_N = glGetUniformLocation(sh_parts.ID, "N");
		
		glUniform1f(loc_N, 256.0f);
		glUniform1i(loc_num_waves, add_waves);
		glUniform1i(loc_num_waves_1, add_waves_1);

		vao_p.Bind();

		glDrawArrays(GL_TRIANGLES, 0, 6);

		if (save) {

				//saveImg("C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\OMIT_IMAG_TEST_FFT_OUTPUT_.png");
				//save = false;
		}
		
		float curr_time = glfwGetTime();
		delta_time += curr_time - last_time;
		last_time = curr_time;
		if (delta_time > 0.0f && add_waves_1 <= 128 && add_waves != 128) {
			delta_time = 0;
			add_waves_1++;
			if (add_waves_1 >= 128) {
				add_waves++;
				add_waves_1 = 0;
			}

			
		}

		if (add_waves == 128) {
			add_waves_1 = 128;
		}

		cout << "wave_lim X = " << add_waves << " wave_lim_1 =" << add_waves_1 <<  '\n';

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


