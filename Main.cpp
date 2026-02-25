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



	



	VAO vaoF;
	vaoF.Bind();

	VBO vboF(plane, sizeof(plane));

	vaoF.linkVBO(vboF, 2, 2);
	vaoF.Unbind();
	vboF.Unbind();

	Texture texF("", true);




	

	/*

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();



		glBindFramebuffer(GL_FRAMEBUFFER, texF.fboID);



		glDisable(GL_DEPTH_TEST);
		glClearColor(0.9f, 0.4f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		sh.Use();

		vao.Bind();

	//	tex.Bind();
		glActiveTexture(GL_TEXTURE4);
		f4.Bind();
		Shader::setUniform(sh.ID, "freq4",(unsigned int) 4);

		glActiveTexture(GL_TEXTURE5);
		f5.Bind();
		Shader::setUniform(sh.ID, "freq5", (unsigned int)5);

		glActiveTexture(GL_TEXTURE1);
		f1.Bind();
		Shader::setUniform(sh.ID, "freq1", (unsigned int)1);
	






		glDrawArrays(GL_TRIANGLES, 0, 6);

		glm::mat4 rot = glm::mat4(1.0f);

		rot = glm::rotate(rot, glm::radians(-14.036f + 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		Shader::setUniform(sh.ID, "rotate", rot);
		Shader::setUniform(sh.ID, "signal.frequency", 16.0f);
		Shader::setUniform(sh.ID, "signal.amplitude", 1.0f);
		Shader::setUniform(sh.ID, "signal.phase", (float) glm::radians(0.0f));

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);



		if (save) {
		
		//	saveImg("C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\frequence_16_4_2d_FFT_test_256.png");
			//save = false;
		}

//		glReadPixels(0, 0, 1024, 1, GL_RGBA, GL_UNSIGNED_BYTE, ren);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		glClearColor(0.9f, 0.4f, 0.2f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT);

		shF.Use();

		vaoF.Bind();

		texF.Bind();
		glActiveTexture(GL_TEXTURE9);
		Shader::setUniform(shF.ID, "filterTexture", (unsigned int) 9);



		glDrawArrays(GL_TRIANGLES, 0, 6);

	

		glfwSwapBuffers(window);
		
	}*/
	fft2D fft(64.0f, 64.0f);
	
	vector<complex<float>> fftInputRow;
	
	cout << "SIZE OF IMAGE INPUT ROW = " << fftInputRow.size() << '\n';
	unsigned char* image_fft;
	string pathToImage = "C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\rectangle_test_fft_256.png";
	int fftHeight, fftWidth, fftNumChannels;
	image_fft = stbi_load(pathToImage.c_str(), &fftWidth, &fftHeight, &fftNumChannels, 0);
	cout << "NUM COLOR CHANNELS  =  " << fftNumChannels << " WIDTH = " << fftWidth << " HEIGHT = " << fftHeight << '\n';
//	cout << "THE SIZE OF LOADED IMAGE ARRAY = " << sizeof(image_fft) << '\n';
	
	vector<vector<complex<float>>> imageData_rows;
//	vector<vector<complex<float>>> imageData_cols;

	int x1 = 0;
	int y1 = 0;


	int sizeImg = fftHeight;
	
	for (int y = 0; y < sizeImg; y++) {
	
		vector<complex<float>> one_row;

		for (int x = 0; x < sizeImg; x ++) {
		
			complex<float> c_img((float)image_fft[y1]/255.0f, 0.0f);
			
			y1 += 3;
			
			one_row.push_back(c_img);

		}

		imageData_rows.push_back(one_row);

	}

	vector<complex<float>> row_test = imageData_rows[0];

	row_test = fft.fft_1D(row_test);

	cout << "SIZE ->AFTER FFT ON ROW DATA = " << row_test.size() << '\n';

	fft2D::printMaxMag(row_test);


	for (int ro = 0; ro < imageData_rows.size(); ro++) {
		
		row_test[ro] = imageData_rows[ro][3];
	//	row_test.push_back(imageData_rows[ro][16]);
		
			
	}
	cout << "SIZE ->AFTER GATHERING COLUMN DATA = " << row_test.size() << '\n';

	row_test = fft.fft_1D(row_test);

	cout << "SIZE ->AFTER FFT ON COLUMN DATA = " << row_test.size() << '\n';

	fft2D::printMaxMag(row_test);

	int n = 0;
	//cout << "sizeof output = " <<  << '\n';

	cout << "NUMBER OF IMAGE ROWS IN VECTOR = " << imageData_rows.size() << '\n';
	
	vector<vector<complex<float>>> image_2D_freq_sprectrum = fft.fft_2D(imageData_rows);

	fft2D::printMaxMag(image_2D_freq_sprectrum);


	glm::vec2 freq_re_im[169];

	glm::vec2 freq_re_im_1[169];

	glm::vec2 freq_re_im_2[169];

	glm::vec2 freq_re_im_3[169];

	glm::vec2 freq_re_im_4[169];

	glm::vec2 freq_re_im_5[169];

	glm::vec2 freq_re_im_6[169];

	glm::vec2 freq_re_im_7[169];


	int wave_count = 0;
	int wave_count_ph_mag = 0;
	/*

	float* fft_fr_x_y_re_im_data = new float[50 * 50 * 4];
	float* fft_phase_mag_data = new float[50 * 50 * 2];

	string debug_tex = "";

	for (int x = 0; x < 50; x++) {

		string one_line = "";
		
		for (int y = 0; y < 50; y++) {
		
	//		fft_fr_x_y_re_im[wave_count + y]

			fft_fr_x_y_re_im_data[wave_count] = ((float) x) / (256.0f * 256.0f);

			fft_fr_x_y_re_im_data[wave_count + 1] = ((float) y)/(256.0f * 256.0f);

			fft_fr_x_y_re_im_data[wave_count + 2] = image_2D_freq_sprectrum[x][y].real() / (256.0f * 256.0f);

			fft_fr_x_y_re_im_data[wave_count + 3] = image_2D_freq_sprectrum[x][y].imag()/ (256.0f * 256.0f);

			fft_phase_mag_data[wave_count_ph_mag] = atan(image_2D_freq_sprectrum[x][y].imag() / image_2D_freq_sprectrum[x][y].real())/(2 * M_PI)/M_PI;

			fft_phase_mag_data[wave_count_ph_mag + 1] = sqrt(pow(image_2D_freq_sprectrum[x][y].imag(), 2.0f) + pow(image_2D_freq_sprectrum[x][y].real(), 2.0f)) / (256.0f * 256.0f);
			
		//	one_line += "FREQ X = " + fft_fr_x_y_re_im_data[wave_count] + " FREQ Y = " + fft_fr_x_y_re_im_data[wave_count + 1] + "\n";


			wave_count_ph_mag += 2;
			wave_count+=4;
		}

	//	debug_tex += one_line;

	}
	
	//cout << debug_tex << '\n';

	
	unsigned int tex_fr_x_y_re_im, tex_phase_mag;


	//---------------- X FREQ, Y FREQ, REAL, IMAGINARY VALUES TEXTURE
	
	glGenTextures(1, &tex_fr_x_y_re_im);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, tex_fr_x_y_re_im);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 50, 50, 0, GL_RGBA, GL_FLOAT, fft_fr_x_y_re_im_data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glGenerateMipmap(GL_TEXTURE_2D);
	
	//---------------- PHASE, MAGNITUDE VALUES TEXTURE

	glGenTextures(1, &tex_phase_mag);


	glBindTexture(GL_TEXTURE_2D, tex_phase_mag);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 3);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, 50, 50, 0, GL_RG, GL_FLOAT, fft_phase_mag_data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glGenerateMipmap(GL_TEXTURE_2D);

	*/


	
	

	for (int x = 0; x < 13; x++) {
	
		for (int y = 0; y < 13; y++) {
		
		//	freq_re_im[wave_count].x = (float) x;
		//	freq_re_im[wave_count].y = (float) y;
	
		//	mag[wave_count] = sqrt(pow(image_2D_freq_sprectrum[x][y].real(), 2.0f) + pow(image_2D_freq_sprectrum[x][y].imag(), 2.0f));

		//	phase[wave_count] = atan2(image_2D_freq_sprectrum[x][y].imag() , image_2D_freq_sprectrum[x][y].real());

			freq_re_im[wave_count].x = image_2D_freq_sprectrum[x][y].real();

			freq_re_im[wave_count].y = image_2D_freq_sprectrum[x][y].imag();

		//	cout << "MAG = " << mag[wave_count] << '\n' << "PHASE = " << phase[wave_count] << "\n FRQ_X = " << freq_X[wave_count] << "\n FRQ_Y = " << freq_Y[wave_count] << '\n';

			wave_count++;

		
		}

	}

	wave_count = 0;

	for (int x = 13; x < 26; x++) {

		for (int y = 13; y < 26; y++) {

		//	freq_re_im_1[wave_count].x = (float)x;
		//	freq_re_im_1[wave_count].y = (float)y;

		//	mag_1[wave_count] = sqrt(pow(image_2D_freq_sprectrum[x][y].real(), 2.0f) + pow(image_2D_freq_sprectrum[x][y].imag(), 2.0f));

		//	phase_1[wave_count] = atan2(image_2D_freq_sprectrum[x][y].imag() , image_2D_freq_sprectrum[x][y].real());

			freq_re_im_1[wave_count].x = image_2D_freq_sprectrum[x][y].real();

			freq_re_im_1[wave_count].y = image_2D_freq_sprectrum[x][y].imag();

		//	cout << "MAG = " << mag[wave_count] << '\n' << "PHASE = " << phase_1[wave_count] << "\n FRQ_X = " << freq_re_im_1[wave_count].x << "\n FRQ_Y = " << freq_re_im_1[wave_count].y << '\n';

			wave_count++;


		}

	}


	wave_count = 0;

	for (int x = 26; x < 39; x++) {

		for (int y = 26; y < 39; y++) {

		//	freq_re_im_2[wave_count].x = (float)x;
		//	freq_re_im_2[wave_count].y = (float)y;

		//	mag_2[wave_count] = sqrt(pow(image_2D_freq_sprectrum[x][y].real(), 2.0f) + pow(image_2D_freq_sprectrum[x][y].imag(), 2.0f));

//			phase_2[wave_count] = atan2(image_2D_freq_sprectrum[x][y].imag() , image_2D_freq_sprectrum[x][y].real());

			freq_re_im_2[wave_count].x = image_2D_freq_sprectrum[x][y].real();

			freq_re_im_2[wave_count].y = image_2D_freq_sprectrum[x][y].imag();

			//cout << "MAG = " << mag[wave_count] << '\n' << "PHASE = " << phase[wave_count] << "\n FRQ_X = " << freq_X[wave_count] << "\n FRQ_Y = " << freq_Y[wave_count] << '\n';

			wave_count++;


		}

	}

	wave_count = 0;

	for (int x = 243; x < 256; x++) {

		for (int y = 0; y < 13; y++) {

			//	freq_re_im_2[wave_count].x = (float)x;
			//	freq_re_im_2[wave_count].y = (float)y;

			//	mag_2[wave_count] = sqrt(pow(image_2D_freq_sprectrum[x][y].real(), 2.0f) + pow(image_2D_freq_sprectrum[x][y].imag(), 2.0f));

	//			phase_2[wave_count] = atan2(image_2D_freq_sprectrum[x][y].imag() , image_2D_freq_sprectrum[x][y].real());

			freq_re_im_3[wave_count].x = image_2D_freq_sprectrum[x][y].real();

			freq_re_im_3[wave_count].y = image_2D_freq_sprectrum[x][y].imag();

			//cout << "MAG = " << mag[wave_count] << '\n' << "PHASE = " << phase[wave_count] << "\n FRQ_X = " << freq_X[wave_count] << "\n FRQ_Y = " << freq_Y[wave_count] << '\n';

			wave_count++;


		}

	}

	wave_count = 0;

	for (int x = 0; x < 13; x++) {

		for (int y = 243; y < 256; y++) {

			//	freq_re_im_2[wave_count].x = (float)x;
			//	freq_re_im_2[wave_count].y = (float)y;

			//	mag_2[wave_count] = sqrt(pow(image_2D_freq_sprectrum[x][y].real(), 2.0f) + pow(image_2D_freq_sprectrum[x][y].imag(), 2.0f));

	//			phase_2[wave_count] = atan2(image_2D_freq_sprectrum[x][y].imag() , image_2D_freq_sprectrum[x][y].real());

			freq_re_im_4[wave_count].x = image_2D_freq_sprectrum[x][y].real();

			freq_re_im_4[wave_count].y = image_2D_freq_sprectrum[x][y].imag();

			//cout << "MAG = " << mag[wave_count] << '\n' << "PHASE = " << phase[wave_count] << "\n FRQ_X = " << freq_X[wave_count] << "\n FRQ_Y = " << freq_Y[wave_count] << '\n';

			wave_count++;


		}

	}

	wave_count = 0;

	for (int x = 243; x < 256; x++) {

		for (int y = 243; y < 256; y++) {

			//	freq_re_im_2[wave_count].x = (float)x;
			//	freq_re_im_2[wave_count].y = (float)y;

			//	mag_2[wave_count] = sqrt(pow(image_2D_freq_sprectrum[x][y].real(), 2.0f) + pow(image_2D_freq_sprectrum[x][y].imag(), 2.0f));

	//			phase_2[wave_count] = atan2(image_2D_freq_sprectrum[x][y].imag() , image_2D_freq_sprectrum[x][y].real());

			freq_re_im_5[wave_count].x = image_2D_freq_sprectrum[x][y].real();

			freq_re_im_5[wave_count].y = image_2D_freq_sprectrum[x][y].imag();

			//cout << "MAG = " << mag[wave_count] << '\n' << "PHASE = " << phase[wave_count] << "\n FRQ_X = " << freq_X[wave_count] << "\n FRQ_Y = " << freq_Y[wave_count] << '\n';

			wave_count++;


		}

	}

	wave_count = 0;

	for (int x = 78; x < 91; x++) {

		for (int y = 78; y < 91; y++) {

			//	freq_re_im_2[wave_count].x = (float)x;
			//	freq_re_im_2[wave_count].y = (float)y;

			//	mag_2[wave_count] = sqrt(pow(image_2D_freq_sprectrum[x][y].real(), 2.0f) + pow(image_2D_freq_sprectrum[x][y].imag(), 2.0f));

	//			phase_2[wave_count] = atan2(image_2D_freq_sprectrum[x][y].imag() , image_2D_freq_sprectrum[x][y].real());

			freq_re_im_6[wave_count].x = image_2D_freq_sprectrum[x][y].real();

			freq_re_im_6[wave_count].y = image_2D_freq_sprectrum[x][y].imag();

			//cout << "MAG = " << mag[wave_count] << '\n' << "PHASE = " << phase[wave_count] << "\n FRQ_X = " << freq_X[wave_count] << "\n FRQ_Y = " << freq_Y[wave_count] << '\n';

			wave_count++;


		}

	}


	wave_count = 0;

	for (int x = 91; x < 104; x++) {

		for (int y = 91; y < 104; y++) {

			//	freq_re_im_2[wave_count].x = (float)x;
			//	freq_re_im_2[wave_count].y = (float)y;

			//	mag_2[wave_count] = sqrt(pow(image_2D_freq_sprectrum[x][y].real(), 2.0f) + pow(image_2D_freq_sprectrum[x][y].imag(), 2.0f));

	//			phase_2[wave_count] = atan2(image_2D_freq_sprectrum[x][y].imag() , image_2D_freq_sprectrum[x][y].real());

			freq_re_im_7[wave_count].x = image_2D_freq_sprectrum[x][y].real();

			freq_re_im_7[wave_count].y = image_2D_freq_sprectrum[x][y].imag();

			//cout << "MAG = " << mag[wave_count] << '\n' << "PHASE = " << phase[wave_count] << "\n FRQ_X = " << freq_X[wave_count] << "\n FRQ_Y = " << freq_Y[wave_count] << '\n';

			wave_count++;


		}

	}
	//cout << "SIZE OF ARRAYS = " << sizeof(phase) << '\n';

	sh.Use();

	unsigned int loc_phase, loc_mag, loc_freq_re_im_vec;
/*
	loc_phase = glGetUniformLocation(sh.ID, "phase");
	glUniform1fv(loc_phase, 169, &phase[0]);

	loc_mag = glGetUniformLocation(sh.ID, "mag");
	glUniform1fv(loc_mag, 169, &mag[0]);
*/
	loc_freq_re_im_vec = glGetUniformLocation(sh.ID, "freq_re_im");
	glUniform2fv(loc_freq_re_im_vec, 169, glm::value_ptr(freq_re_im[0]));

	



	//--------------

	unsigned int loc_phase_1, loc_mag_1, loc_freq_re_im_vec_1;
/*
	loc_phase_1 = glGetUniformLocation(sh.ID, "phase_1");
	glUniform1fv(loc_phase_1, 169, &phase_1[0]);

	loc_mag_1 = glGetUniformLocation(sh.ID, "mag_1");
	glUniform1fv(loc_mag_1, 169, &mag_1[0]);
*/
	loc_freq_re_im_vec_1 = glGetUniformLocation(sh.ID, "freq_re_im_1");
	glUniform2fv(loc_freq_re_im_vec_1, 169, glm::value_ptr(freq_re_im_1[0]));

	//--------------

	unsigned int loc_phase_2, loc_mag_2, loc_freq_re_im_vec_2;
/*
	loc_phase_2 = glGetUniformLocation(sh.ID, "phase_2");
	glUniform1fv(loc_phase_2, 169, &phase_2[0]);

	loc_mag_2 = glGetUniformLocation(sh.ID, "mag_2");
	glUniform1fv(loc_mag_2, 169, &mag_2[0]);
*/
	loc_freq_re_im_vec_2 = glGetUniformLocation(sh.ID, "freq_re_im_2");
	glUniform2fv(loc_freq_re_im_vec_2, 169, glm::value_ptr(freq_re_im_2[0]));

	unsigned int loc_freq_re_im_vec_3;

	loc_freq_re_im_vec_3 = glGetUniformLocation(sh.ID, "freq_re_im_3");
	glUniform2fv(loc_freq_re_im_vec_3, 169, glm::value_ptr(freq_re_im_3[0]));

	unsigned int loc_freq_re_im_vec_4;

	loc_freq_re_im_vec_4 = glGetUniformLocation(sh.ID, "freq_re_im_4");
	glUniform2fv(loc_freq_re_im_vec_4, 169, glm::value_ptr(freq_re_im_4[0]));

	unsigned int loc_freq_re_im_vec_5;

	loc_freq_re_im_vec_5 = glGetUniformLocation(sh.ID, "freq_re_im_5");
	glUniform2fv(loc_freq_re_im_vec_5, 169, glm::value_ptr(freq_re_im_5[0]));

	unsigned int loc_freq_re_im_vec_6;

	loc_freq_re_im_vec_6 = glGetUniformLocation(sh.ID, "freq_re_im_6");
//	glUniform2fv(loc_freq_re_im_vec_6, 169, glm::value_ptr(freq_re_im_6[0]));


	unsigned int loc_freq_re_im_vec_7;

	loc_freq_re_im_vec_7 = glGetUniformLocation(sh.ID, "freq_re_im_7");
//	glUniform2fv(loc_freq_re_im_vec_7, 169, glm::value_ptr(freq_re_im_7[0]));

	
	

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		glBindFramebuffer(GL_FRAMEBUFFER, texF.fboID);


		glDisable(GL_DEPTH_TEST);
		glClearColor(0.9f, 0.4f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		sh.Use();

		vao.Bind();
		/*

		glBindTexture(GL_TEXTURE_2D, tex_fr_x_y_re_im);
		glActiveTexture(GL_TEXTURE10);
		int loc_fr_complex = glGetUniformLocation(sh.ID, "frq_x_y_re_im");
		glUniform1i(loc_fr_complex, 10);

		glBindTexture(GL_TEXTURE_2D, tex_phase_mag);
		glActiveTexture(GL_TEXTURE11);
		int loc_phase = glGetUniformLocation(sh.ID, "frq_phase_mag");
		glUniform1i(loc_phase, 11);
		*/

		glDrawArrays(GL_TRIANGLES, 0, 6);

	


		if (save) {

				saveImg("C:\\Users\\Toms\\Desktop\\OpenGL\\FourierTransform\\INVERSE_rect_orig_13.png");
				//save = false;
		}


		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		glClearColor(0.9f, 0.4f, 0.2f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT);

		shF.Use();

		vaoF.Bind();

		texF.Bind();
		glActiveTexture(GL_TEXTURE9);
		Shader::setUniform(shF.ID, "filterTexture", (unsigned int)9);



		glDrawArrays(GL_TRIANGLES, 0, 6);


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


