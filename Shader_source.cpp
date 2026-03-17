#include "Shader_source.h"



string shader_source::getFileContent(const char* file_path) {

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

