#ifndef SHADER_H
#define SHADER_H


#include <glad/glad.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::string;

enum fft_orientation {

	HORIZONTAL,
	VERTICAL

};


class shader_source {
	public:



		static fft_orientation orientation;
public:
		unsigned int ID;
		shader_source() = delete;
		shader_source(const shader_source&) = delete;
		shader_source operator=(const shader_source&) = delete;

		shader_source(string file_path, GLenum shader_type);
		shader_source(string file_path, GLenum shader_type, fft_orientation orientation, int num_samples, int samples_per_processor);

		static string get_file_content(const char* path);

		static string set_compute_shader_values(string source, int num_samples, int samples_per_processor, fft_orientation orientation);
		

};

#endif