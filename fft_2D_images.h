#ifndef FFT_H
#define FFT_H

#define M_PI 3.1415926535897932385

#include <iostream>
#include <complex>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <cmath>
#include <iterator>
#include <algorithm>


using std::vector;
using std::complex;

class Wave;
//class 2D_fft;
using std::cout, std::endl;


class fft2D{

public:
	vector<complex<float>> waves;
	float width;
	float height;

	fft2D(float width_W, float height_H) : width{ width_W }, height{height_H} {
		waves.reserve((int)height_H * (int)width_W);
	}

	fft2D() {}

	vector<complex<float>> fft_1D(vector<complex<float>> signal);

	vector<vector<complex<float>>> fft_2D(vector<vector<complex<float>>> signalR);

	static void printMaxMag(vector<vector<complex<float>>> fft_2D_output);


};


class Wave {

public:
	complex<float> fftOutput;
	float frequencyH;
	float frequencyV;
	float phase;
	float magnitude;
	float* heightImg;
	float* widthImg;

	Wave(complex<float>& fft_Output, float frequency_H, float frequency_V,float &height,float &width) : fftOutput{fft_Output},
																							frequencyH{ frequency_H },
																								frequencyV{ frequency_V },
																								heightImg{ &height },
																								widthImg{ &width } {

		setMembers();

	}
	

	void setMembers() {

		phase = atan(fftOutput.imag() / fftOutput.real());
		magnitude = sqrt(pow(fftOutput.real(), 2.0f) + pow(fftOutput.imag(), 2.0f)) / ((*widthImg) * (*heightImg));
		}

};
	


#endif