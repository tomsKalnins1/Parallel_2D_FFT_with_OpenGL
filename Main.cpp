#include <stdio.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <math.h>
#include <string>


#include "ShaderProgram.h"
#include "VAO.h"
#include "VBO.h"
#include "Texture.h"


#define THE_PI 3.1415926535897932385

using std::cout, std::string, std::vector;

float last_time = 0;
float delta_time = 0;

bool save = true;

struct Data {

	unsigned int bits;
	int forward;
	int orientation;
	float norm;

};


void saveImg(string path) {

	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * 1024;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * 1024;
	vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, 1024, 1024, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	stbi_write_png(path.c_str(), 1024, 1024, 3, buffer.data(), stride);

}

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

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	string v = "vert.vs";
	string f = "frag_img_anim_ifft.fs";
	
	string source_frag = "source_img_frag.fs";

	string parts_frag = "freq_comp_frag.fs";
	
	glViewport(0, 0, 1024, 1024);

	//---------------------------------------------------------------------------------DISPLAY ANIAMTED IMAGE ASSEMBLEY

	ShaderProgram sh_anim_asm(v.c_str(), f.c_str());

	VAO vao;
	vao.bind_VAO();
	VBO vbo(plane, sizeof(plane));
	vao.link_VBO(vbo, 2, 2);
	vao.unbind_VAO();
	vbo.unbind_VBO();

	//---------------------------------------------------------------------------------DISPLAY ORIGINAL IMAGE

	ShaderProgram shF(v.c_str(), source_frag.c_str());

	VAO vao_s;
	vao_s.bind_VAO();
	VBO vbo_s(plane_img, sizeof(plane_img));
	vao_s.link_VBO(vbo_s, 2, 2);
	vao_s.unbind_VAO();
	vbo_s.unbind_VBO();

	//---------------------------------------------------------------------------------DISPLAY INDIVIDUAL FREQUENCIES

	ShaderProgram sh_parts(v.c_str(), parts_frag.c_str());

	VAO vao_p;
	vao_p.bind_VAO();
	VBO vbo_p(plane_parts, sizeof(plane_parts));
	vao_p.link_VBO(vbo_p, 2, 2);
	vao_p.unbind_VAO();
	vbo_p.unbind_VBO();

	//---------------------------------------------------------------------------------DISPLAY THE FFT OUTPUT

	VAO vao_out;
	vao_out.bind_VAO();
	VBO vbo_out(fft_texture_output, sizeof(fft_texture_output));
	vao_out.link_VBO(vbo_out, 2, 2);
	vao_out.unbind_VAO();
	vbo_out.unbind_VBO();



	//---------------------------------------------------------------------------------FFT ON ROWS

	float start_time = glfwGetTime();

	ShaderProgram compute_prog_h_0("fft_compute_horizontal.cs", 256, 4);

	string pathToImage = "C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\Test_images\\cameraman.png";

	Texture input_img(GL_RGBA32F, GL_RGBA, pathToImage, 256, 256);

	Texture first_fft = Texture{};
	Texture second_fft = Texture{};
	Texture output_1 = Texture{};
	Texture output_2 = Texture{};

	Texture::activate_tex_unit(0);
	input_img.bind_texture();
	input_img.bind_image_2D(0);



	Texture::activate_tex_unit(1);
	first_fft.bind_texture();
	first_fft.bind_image_2D(1);

	unsigned int n_bits = ShaderProgram::num_bits((unsigned int)input_img.height);
	Data ssbo_fft_data_H = { n_bits, -1, 0, 1.0f };
	Data ssbo_fft_data_V = { n_bits, -1, 1, 1.0f };

	unsigned int buffer_comp_h_0;
	glCreateBuffers(1, &buffer_comp_h_0);
	glNamedBufferStorage(buffer_comp_h_0, sizeof(Data), &ssbo_fft_data_H, GL_DYNAMIC_STORAGE_BIT);	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer_comp_h_0);
	
	compute_prog_h_0.use_shader_prog();
	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256 / 1), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	//---------------------------------------------------------------------------------TRANSPOSE

	string path_trans_v = "transpose.cs";

	ShaderProgram rot(path_trans_v.c_str(), 256, 4);

	Texture::activate_tex_unit(7);
	first_fft.bind_texture();
	first_fft.bind_image_2D(0);

	Texture::reset_to_base(input_img);
	Texture::activate_tex_unit(8);
	input_img.bind_texture();
	input_img.bind_image_2D(1);

	rot.use_shader_prog();
	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//---------------------------------------------------------------------------------FFT ON COLUMNS

	//ShaderProgram compute_prog_h_1("fft_compute_horizontal.cs", HORIZONTAL, 256, 4);

	Texture::activate_tex_unit(0);
	input_img.bind_texture();
	input_img.bind_image_2D(0);


	Texture::activate_tex_unit(1);
	second_fft.bind_texture();
	second_fft.bind_image_2D(1);


	unsigned int buffer_comp_h_1;
	glCreateBuffers(1, &buffer_comp_h_1);
	glNamedBufferStorage(buffer_comp_h_1, sizeof(Data), &ssbo_fft_data_V, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer_comp_h_1);

	compute_prog_h_0.use_shader_prog();

	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256 / 1), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//---------------------------------------------------------------------------------TRANSPOSE

	Texture::activate_tex_unit(7);
	second_fft.bind_texture();
	second_fft.bind_image_2D(0);


	Texture::activate_tex_unit(8);
	output_1.bind_texture();
	output_1.bind_image_2D(1);


	rot.use_shader_prog();
	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	float end_time = glfwGetTime();

	float fft_compute_time = end_time - start_time;

	cout << "TIME FOR 256X256 IMG FFT = " << fft_compute_time << " seconds \n";

	//---------------------------------------------------------------------------------FFT COMPLETE


	//---------------------------------------------------------------------------------INVERSE FFT WITH COMPUTE SHADERS
	
	//---------------------------------------------------------------------------------IFFT ON ROWS

	//ShaderProgram compute_prog_h_2("fft_compute_horizontal.cs", HORIZONTAL, 256, 4);

	start_time = glfwGetTime();

	Texture fft_1(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);
	Texture fft_1_transp(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);
	Texture fft_2(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);
	Texture ifft(GL_RGBA32F, GL_RGBA, "no_file", 256, 256);

	//output_1.ID = fft_2d_output_id;

	Texture::activate_tex_unit(0);
	output_1.bind_texture();
	output_1.bind_image_2D(0);

	Texture::reset_to_base(first_fft);
	Texture::activate_tex_unit(1);
	first_fft.bind_texture();
	first_fft.bind_image_2D(1);

	Data ssbo_ifft_data_H = { n_bits, 1, 1, 1.0f };
	Data ssbo_ifft_data_V = { n_bits, 1, 1, 255.0f * 255.0f };

	unsigned int buffer_comp_h_2;
	glCreateBuffers(1, &buffer_comp_h_2);
	glNamedBufferStorage(buffer_comp_h_2, sizeof(Data), &ssbo_ifft_data_H, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer_comp_h_2);

	compute_prog_h_0.use_shader_prog();
	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256 / 1), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//---------------------------------------------------------------------------------TRANSPOSE

	Texture::activate_tex_unit(7);
	first_fft.bind_texture();
	first_fft.bind_image_2D(0);


	Texture::activate_tex_unit(8);
	output_2.bind_texture();
	output_2.bind_image_2D(1);


	rot.use_shader_prog();
	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//---------------------------------------------------------------------------------IFFT ON COLUMNS

	//ShaderProgram compute_prog_h_3("fft_compute_horizontal.cs", HORIZONTAL, 256, 4);

	Texture::activate_tex_unit(0);
	output_2.bind_texture();
	output_2.bind_image_2D(0);

	Texture::reset_to_base(second_fft);
	Texture::activate_tex_unit(1);
	second_fft.bind_texture();
	second_fft.bind_image_2D(1);

	unsigned int buffer_comp_h_3;
	glCreateBuffers(1, &buffer_comp_h_3);
	glNamedBufferStorage(buffer_comp_h_3, sizeof(Data), &ssbo_ifft_data_V, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer_comp_h_3);

	compute_prog_h_0.use_shader_prog();
	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256 / 1), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//---------------------------------------------------------------------------------TRANSPOSE

	Texture::activate_tex_unit(7);
	second_fft.bind_texture();
	second_fft.bind_image_2D(0);

	Texture::reset_to_base(output_2);
	Texture::activate_tex_unit(8);
	output_2.bind_texture();
	output_2.bind_image_2D(1);


	rot.use_shader_prog();
	glDispatchCompute((unsigned int)ceil(1), (unsigned int)ceil(256), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	end_time = glfwGetTime();

	fft_compute_time = end_time - start_time;

	cout << "TIME FOR 256X256 IMG IFFT = " << fft_compute_time << " seconds \n";

	//---------------------------------------------------------------------------------IFFT COMPLETE

	glm::mat4 trans = glm::mat4(1.0f);

	int add_waves = 0;
	int add_waves_1 = 0;

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		glClearColor(0.156f, 0.427f, 0.482f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		sh_anim_asm.use_shader_prog();
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		unsigned int loc_num_waves_1 = glGetUniformLocation(sh_anim_asm.ID, "wave_lim_1");
		unsigned int loc_num_waves = glGetUniformLocation(sh_anim_asm.ID, "wave_lim_0");
		unsigned int loc_N = glGetUniformLocation(sh_anim_asm.ID, "N");
		glUniform1f(loc_N, 256.0f);
		
		glActiveTexture(GL_TEXTURE5);

		//---------------------------------------------------------------------------------ANIMATED IMAGE ASSEMBLEY FROM INDIVIDUAL FREQUENCIES

		output_1.bind_texture();

		int location_tex = glGetUniformLocation(sh_anim_asm.ID, "real_imag");
		
		glUniform1i(location_tex, 5);
		trans = glm::mat4(1.0f);
		trans = glm::translate(trans, glm::vec3(-0.5f, 0.5f, 0.0f));
		trans = glm::scale(trans, glm::vec3(0.8f, 0.8f, 0.0f));

		unsigned int loc_move = glGetUniformLocation(sh_anim_asm.ID, "move");

		glUniformMatrix4fv(loc_move, 1, GL_FALSE, glm::value_ptr(trans));
		ShaderProgram::set_uniform(sh_anim_asm.ID, "move", trans);

		glUniform1f(loc_N, 256.0f);
		glUniform1i(loc_num_waves, add_waves);
		glUniform1i(loc_num_waves_1, add_waves_1);
		vao.bind_VAO();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//--------------------------------------------------------------------------------- THE IFFT OUTPUT OF AN IMAGE WITH COMPUTE SHADERS
		
		shF.use_shader_prog();
		glActiveTexture(GL_TEXTURE5);
		output_2.bind_texture();
		trans = glm::mat4(1.0f);

		trans = glm::translate(trans, glm::vec3(0.5f, 0.5f, 0.0f));

		trans = glm::scale(trans, glm::vec3(0.8f, 0.8f, 0.0f));


		ShaderProgram::set_uniform(shF.ID, "move", trans);
		ShaderProgram::set_uniform(shF.ID, "filterTexture",(unsigned int) 5);

		vao_s.bind_VAO();

		glDrawArrays(GL_TRIANGLES, 0, 6);

		//---------------------------------------------------------------------------------THE FFT OUTPUT 

		glActiveTexture(GL_TEXTURE6);
		output_1.bind_texture();

		trans = glm::mat4(1.0f);

	
		trans = glm::translate(trans, glm::vec3(0.5f, -0.4f, 0.0f));

		trans = glm::scale(trans, glm::vec3(0.8f, 0.8f, 0.0f));



		ShaderProgram::set_uniform(shF.ID, "move", trans);
		ShaderProgram::set_uniform(shF.ID, "filterTexture", (unsigned int)6);

		vao_out.bind_VAO();
		
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//---------------------------------------------------------------------------------EVERY FREQUENCY COMPONENTS FOR DISPLAY
		
		sh_parts.use_shader_prog();
		glActiveTexture(GL_TEXTURE7);
		output_1.bind_texture();

		ShaderProgram::set_uniform(sh_parts.ID, "real_imag", (unsigned int)7);

		trans = glm::mat4(1.0f);

		trans = glm::translate(trans, glm::vec3(-0.5f, -0.4f, 0.0f));

		trans = glm::scale(trans, glm::vec3(0.8f, 0.8f, 0.0f));

		ShaderProgram::set_uniform(sh_parts.ID, "move", trans);

		loc_num_waves = glGetUniformLocation(sh_parts.ID, "wave_lim_0");
		loc_num_waves_1 = glGetUniformLocation(sh_parts.ID, "wave_lim_1");
		loc_N = glGetUniformLocation(sh_parts.ID, "N");
		
		glUniform1f(loc_N, 256.0f);
		glUniform1i(loc_num_waves, add_waves);
		glUniform1i(loc_num_waves_1, add_waves_1);

		vao_p.bind_VAO();

		glDrawArrays(GL_TRIANGLES, 0, 6);

		if (save) {

			//saveImg("C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\FFT_OUTPUT_DATA_POWER_SPECTRUM.png");
	
		}
		
		float curr_time = glfwGetTime();
		delta_time += curr_time - last_time;
		last_time = curr_time;
		if (delta_time > 0.0f && add_waves_1 <= 128 && add_waves != 128) {
			delta_time = 0;
			add_waves_1+=3;
			if (add_waves_1 >= 128) {
				add_waves+=1;
				add_waves_1 = 0;
			}

			
		}

		if (add_waves == 128) {
			add_waves_1 = 128;
		}

		glfwSwapBuffers(window);


	
	}
	

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;

}




