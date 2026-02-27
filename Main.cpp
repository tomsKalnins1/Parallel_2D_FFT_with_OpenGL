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

vector<complex<float>> fftOutput(256);

float deltaTime = 0.0f;
float lastTime = 0.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0, -1.0f);
glm::vec3 upDirection = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 moveDown = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 moveUp = glm::vec3(0.0f, 0.0f, 0.0f);

float pitch = 0.0f; //around x
float yaw = -90.0f; //arounf y
//float roll  = 0.0f; //around z

float lastX = 500.0f;
float lastY = 500.0f;

bool mouseFirstMove = true; //the first move of mouse resuls in too large offset => causes jump when the mouse enters the window 


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

void processInput(GLFWwindow* window) {

	float currTime = glfwGetTime();
	deltaTime = currTime - lastTime;
	lastTime = currTime;

	const float cameraSpeed = 1.0f * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {

		cameraPos += cameraFront * cameraSpeed;
		cout << "w";
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {

		cameraPos -= cameraFront * cameraSpeed;

	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {

		cameraPos += glm::normalize(glm::cross(cameraFront, upDirection)) * cameraSpeed;

	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {

		cameraPos -= glm::normalize(glm::cross(cameraFront, upDirection)) * cameraSpeed;

	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {

		cameraPos += upDirection * cameraSpeed;

	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {

		cameraPos -= upDirection * cameraSpeed;

	}

}

void mousePositionCallBack(GLFWwindow* window, double xPos, double yPos) {

	if (mouseFirstMove) {

		lastX = xPos;
		lastY = yPos;

		mouseFirstMove = false;

	}

	float xOffset = xPos - lastX;
	float yOffset = lastY - yPos; //flipped because the y axis for screens is filpped, so need to be reverted to use for regular coordinate system

	lastX = xPos;
	lastY = yPos;

	float sensitivity = 0.01f;

	yaw += xOffset * sensitivity;
	pitch += yOffset * sensitivity;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	cameraFront = glm::normalize(direction);


}


bool save = true;

int main() {

	//	cout << "EXTERN FUNCTION BEING CALLED" << endl;
	//	string fragFile = getFileContent("fragFrameBuffer.fs");
	//	cout << "FILE CONTENT FROM EXTERN FUNCTION : \n " << fragFile << " \t END OF FILE" << endl;

	//	cout << "VECTOR_TO_STRING  = " << glm::to_string(cameraPos) << endl;

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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

	Shader sh(v.c_str(), f.c_str());

	Shader shF(frame_b_vert.c_str(), frabe_b_frag.c_str());

	VAO vao;

	vao.Bind();

	VBO vbo(plane, sizeof(plane));

	vao.linkVBO(vbo, 2, 2);

	vao.Unbind();
	vbo.Unbind();

	Texture tex("image.png", false);



	

	fft2D fft(256.0f, 256.0f);
	
	vector<complex<float>> fftInputRow;
	
	cout << "SIZE OF IMAGE INPUT ROW = " << fftInputRow.size() << '\n';
	unsigned char* image_fft;
	string pathToImage = "C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\frequence_3_3_2d_FFT_test.png";
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
			
			y1 += 4;
			
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



	
	

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		glClearColor(0.9f, 0.4f, 0.2f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT);

		sh.Use();
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fft_output_texture);


		int location_tex = glGetUniformLocation(sh.ID, "real_imag");
		glUniform1i(location_tex, 1);

		vao.Bind();
	

		glDrawArrays(GL_TRIANGLES, 0, 6);

	


		if (save) {

				saveImg("C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\INVERSE_rect_orig_16.png");
				//save = false;
		}
		
		

		glfwSwapBuffers(window);

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


