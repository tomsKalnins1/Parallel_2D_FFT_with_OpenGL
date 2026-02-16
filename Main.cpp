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

vector<complex<float>> fftOutput(1024);

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
	GLsizei stride = nrChannels * 1024;
	stride += (stride % 4) ? (4 - stride % 4) : 0; //make sure the stride is a multiple of 4
	GLsizei bufferSize = stride * 1024;
	vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, 1024, 1024, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
//	cout << "SIZE OF THE IMAGE VECTOR (FOR SAVED IMAGE) = " << buffer.size() << '\n';
	stbi_flip_vertically_on_write(true);
	stbi_write_png(path.c_str(), 1024, 1024, 3, buffer.data(), stride);

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

	GLFWwindow* window = glfwCreateWindow(1024, 1024, "Framebuffer", NULL, NULL);

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

	Shader sh(v.c_str(), f.c_str());

	VAO vao;

	vao.Bind();

	VBO vbo(plane, sizeof(plane));

	vao.linkVBO(vbo, 2, 2);

	vao.Unbind();
	vbo.Unbind();

	Texture tex("image.png", false);

	glm::mat4 rot = glm::mat4(1.0f);

	rot = glm::rotate(rot, (float) glm::radians(0.0), glm::vec3(0.0f, 0.0f, 1.0f));

	Shader::setUniform(sh.ID, "rotate", rot);
	Shader::setUniform(sh.ID, "signal.frequency",  static_cast<unsigned int>(4));
	Shader::setUniform(sh.ID, "signal.amplitude", 0.5f);

	string fBufferVert = "vertFrameBuffer.vs";
	string fBufferFrag = "fragFrameBuffer.fs";

	Shader shF(fBufferVert.c_str(), fBufferFrag.c_str());

	VAO vaoF;
	vaoF.Bind();

	VBO vboF(plane, sizeof(plane));

	vaoF.linkVBO(vboF, 2, 2);
	vaoF.Unbind();
	vboF.Unbind();

	Texture texF("", true);



	GLubyte ren[1024*4];

	Texture f4("frequence_4.png", false);
	Texture f5("frequence_5.png", false);
	Texture f1("frequence_1.png", false);


	

	

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();



		glBindFramebuffer(GL_FRAMEBUFFER, texF.fboID);



		glDisable(GL_DEPTH_TEST);
		glClearColor(0.9f, 0.4f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		sh.Use();

		vao.Bind();

	//	tex.Bind();
	/*	glActiveTexture(GL_TEXTURE4);
		f4.Bind();
		Shader::setUniform(sh.ID, "freq4",(unsigned int) 4);

		glActiveTexture(GL_TEXTURE5);
		f5.Bind();
		Shader::setUniform(sh.ID, "freq5", (unsigned int)5);

		glActiveTexture(GL_TEXTURE1);
		f1.Bind();
		Shader::setUniform(sh.ID, "freq1", (unsigned int)1);
	*/






		glDrawArrays(GL_TRIANGLES, 0, 6);

		glm::mat4 rot = glm::mat4(1.0f);

		rot = glm::rotate(rot, glm::radians(-14.036f + 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		Shader::setUniform(sh.ID, "rotate", rot);
		Shader::setUniform(sh.ID, "signal.frequency", 16.0f);
		Shader::setUniform(sh.ID, "signal.amplitude", 1.0f);
		Shader::setUniform(sh.ID, "signal.phase", (float) glm::radians(0.0f));

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


		/*The freambuffer that is bound at the start of the loop is the one that contains a texture attachment that can be sampled and
		changed with shaders, while also it has render buffer object attached which has also the image to be rendered and all the 
		pixel values, but in a native opengl format and data from it cannot be read from directly.It having this native format makes
		the swappign of buffers faster and helps with optimization. 
		The texture that has an empty array has the scene rendered to it which is then used as texture to render on
		a plane for postprocessing, which is another attachment to the custom frame buffer before the buffers are swapped
		nd just after the scene is is drawn the glReadPixels is called and it read the screen output
		data as image form the*/


		if (save) {
		
			//saveImg("C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\frequence_16_4_2d_FFT_test.png");
			//save = false;
		}

//		glReadPixels(0, 0, 1024, 1, GL_RGBA, GL_UNSIGNED_BYTE, ren);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		glClearColor(0.9f, 0.4f, 0.2f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT);

		shF.Use();

		vaoF.Bind();

		texF.Bind();
		glActiveTexture(GL_TEXTURE10);
		Shader::setUniform(shF.ID, "filterTexture", (unsigned int) 10);



		glDrawArrays(GL_TRIANGLES, 0, 6);


		/*If one thing fails anything, here it was linking due to mismathch of one letter for vertex shader otuput and
		frag shader input -> the program did not link -> it could not be activated -> the framebuffer content
		was rendered to a texture which could then be drawn onto the plane(screen), but the previosu shader program was still running
		which means that the matrix transformations applied to the cube got applied to the plane*/
	//!	shF.Use();

	//!	vaoF.Bind();
		//	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		//	glClear(GL_COLOR_BUFFER_BIT);
		//	glUniform1i(glGetUniformLocation(shF.ID, "filterTexture"), 0);



	//!	texF.Bind();



		//	glUseProgram(0);
		//	glUseProgram(shFID);
		//!	glDrawArrays(GL_TRIANGLES, 0, 6);
	

		glfwSwapBuffers(window);
		
	}
	fft2D fft(64.0f, 64.0f);
	
	vector<complex<float>> fftInputRow;
	
	cout << "SIZE OF IMAGE INPUT ROW = " << fftInputRow.size() << '\n';
	unsigned char* image_fft;
	string pathToImage = "C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\frequence_4_2_2d_FFT_test.png";
	int fftHeight, fftWidth, fftNumChannels;
	image_fft = stbi_load(pathToImage.c_str(), &fftWidth, &fftHeight, &fftNumChannels, 0);
	cout << "NUM COLOR CHANNELS  =  " << fftNumChannels << " WIDTH = " << fftWidth << " HEIGHT = " << fftHeight << '\n';
//	cout << "THE SIZE OF LOADED IMAGE ARRAY = " << sizeof(image_fft) << '\n';
	
	vector<vector<complex<float>>> imageData_rows;
//	vector<vector<complex<float>>> imageData_cols;

	int x1 = 0;
	int y1 = 0;



	
	for (int y = 0; y < 32; y++) {
	
		vector<complex<float>> one_row;

		for (int x = 0; x < 32; x ++) {
		
			complex<float> c_img((float)image_fft[y1], 0.0f);
			
			y1 += 3;
			
			one_row.push_back(c_img);

		}

		imageData_rows.push_back(one_row);

	}
/*
	for (int x = 0; x < 1024; x++) {

		vector<complex<float>> one_col;

		for (int y = 0; y < 1024; y++) {

			complex<float> c_img((float)image_fft[y * 3072 + x1], 0.0f);

			

			one_col.push_back(c_img);

		}
		x1 += 3;
		imageData_cols.push_back(one_col);

	}
	*/
	int n = 0;
	//cout << "sizeof output = " <<  << '\n';

	cout << "NUMBER OF IMAGE ROWS IN VECTOR = " << imageData_rows.size() << '\n';
	
	vector<vector<complex<float>>> image_2D_freq_sprectrum = fft.fft_2D(imageData_rows);

	fft2D::printMaxMag(image_2D_freq_sprectrum);

	/*

	fftOutput = fft.fft_1D(fftInputRow);


	float mag = 0;

	float maxMag = 0;
	int maxInt = 0;
	for (int k = 0; k < 1024; k++) {
		
		//cout << fftOutput[k].real() << " + " << fftOutput[k].imag() << " * (i)" << '\n';
		mag = sqrt(pow(fftOutput[k].imag(), 2.0f) + pow(fftOutput[k].real(), 2.0f));
		//cout << "magnitude of " << k << " 'th  frequency = " << mag / 256.0f << '\n';

		if (mag > maxMag && k != 0) {
			maxMag = mag;
			maxInt = k;
		
		}


	}
	//cout << "sizeof output = " << sizeof(fftOutput) << '\n';

	cout << " FREQUENCY SPECTRUM PEAK = " << maxMag/512.0f + 128.0f << "  AT FREQ.  = " << maxInt << '\n';
	*/

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


