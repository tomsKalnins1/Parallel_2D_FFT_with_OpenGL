#include "Shader_source.h"

shader_source::shader_source(string file_path, GLenum shader_type) {

	string shader_source = get_file_content(file_path.c_str());

	const char* shader_source_char = shader_source.c_str();

	ID = glCreateShader(shader_type);
	glShaderSource(ID, 1, &shader_source_char, NULL);
	glCompileShader(ID);

	GLint compiled;
	glGetShaderiv(ID, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		char errorLog[1024];
		glGetShaderInfoLog(ID, 1024, NULL, errorLog);
		std::cout << ("RENDER PIPELINE SHADER " + file_path + " \n COMPILATION FAILED : \n") << errorLog << '\n';
	}


}

shader_source::shader_source(string file_path, GLenum shader_type, fft_orientation orientation, int num_samples, int samples_per_processor) {
	
	string shader_source = get_file_content(file_path.c_str());

	shader_source = set_compute_shader_values(shader_source, num_samples, samples_per_processor, orientation);

	const char* shader_source_char = shader_source.c_str();

	ID = glCreateShader(shader_type);
	glShaderSource(ID, 1, &shader_source_char, NULL);
	glCompileShader(ID);

	GLint compiled;
	glGetShaderiv(ID, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		char errorLog[1024];
		glGetShaderInfoLog(ID, 1024, NULL, errorLog);
		std::cout << ("COMPUTE SHADER " + file_path + " \n COMPILATION FAILED : \n") << errorLog << '\n';
	}

}


string shader_source::get_file_content(const char* file_path) {

	std::ifstream in(file_path, std::ios::binary);

	if (in) {
		

		string content;

		in.seekg(0, std::ios::end);
		content.resize(in.tellg());
		
		in.seekg(0, std::ios::beg);
		in.read(&content[0], content.size());

		in.close();
		
		return content;
	}

	throw(errno);

}


string shader_source::set_compute_shader_values(string shader_source_file, int num_samples, int samples_per_processor, fft_orientation orientation) {

	string shader_file = "";
	string invocation_orient = "";
	string samples = "NUM_SAMPLES";

	if (orientation == HORIZONTAL) {
		invocation_orient = "X_INVOCATIONS";
	}
	else {
		invocation_orient = "Y_INVOCATIONS";
	}

	

	int index = shader_source_file.find(invocation_orient);

	shader_source_file = shader_source_file.replace(index, invocation_orient.size(), std::to_string(num_samples / samples_per_processor));

	while (shader_source_file.find(samples) < shader_source_file.size()) {

		index = shader_source_file.find(samples);

		shader_source_file = shader_source_file.replace(index, samples.size(), std::to_string(num_samples));

	}

	return shader_source_file;

}