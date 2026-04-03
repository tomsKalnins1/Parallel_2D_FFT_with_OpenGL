#ifndef FFT_H
#define FFT_H

#define M_PI 3.1415926535897932385

#include <iostream>
#include <complex>
#include <vector>
#include <string>
#include <cmath>
#include <thread>
#include <barrier>
#include <mutex>



using std::vector;
using std::complex;
using std::cout, std::endl;


class fft2D{

public:

	static vector<complex<float>> fft_1D(vector<complex<float>> signal, int dir);
	static vector<vector<complex<float>>> fft_2D(vector<vector<complex<float>>> signalR, int dir);

};

	


#endif