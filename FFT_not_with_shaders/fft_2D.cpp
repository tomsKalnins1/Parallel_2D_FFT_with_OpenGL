#include "fft_2D.h"




vector<complex<float>> fft2D::fft_1D(vector<complex<float>> signal, int dir) {

	int len = signal.size();

	if (len <= 1) {

		return signal;

	}

	vector<complex<float>> oddTerms(len/2);
	vector<complex<float>> evenTerms(len/2);

	vector<complex<float>> odd(len / 2);
	vector<complex<float>> even(len / 2);
	
	int n = 0;
	std::string oddStr = "";
	std::string evenStr = "";
	for (int i = 1; i < len; i += 2) {
	
		odd[n] = signal[i];
		n++;

	}

	n = 0;

	for (int k = 0; k < len; k += 2) {

		even[n] = signal[k];
		n++;
	}

	oddTerms = fft_1D(odd, dir);
	evenTerms = fft_1D(even, dir);

	vector<complex<float>> components(len);
	int m = 0;

	for (int j = 0; j < len/2; j++) {

		complex<float> twiddle = std::complex<float>(cos(M_PI * 2.0f * j / len), (dir) * sin(M_PI * 2.0f  * j/ len));
		components[j] = evenTerms[j] + twiddle * oddTerms[j];
		components[j + len / 2] = evenTerms[j] - twiddle * oddTerms[j];
		m = j;

	}

	return components;

}

int reverseBits(unsigned int n) {
	int ans = 0;
	int i = 0;

	while (n > 0 && i < 32) {

		ans <<= 1;

		if (n & 1 == 1){

			ans |= 1;

		}

		n >>= 1;

		i++;

	}

	return ans;

}



vector<vector<complex<float>>> fft2D::fft_2D(vector<vector<complex<float>>> signalRow, int dir) {
	int size = signalRow.size();
	vector<vector<complex<float>>> image_rows_fft(size);
	vector<vector<complex<float>>> fft_2D_output(size);

	int r = 0;
	for (int y = 0; y < size; y++) {

		vector<complex<float>> one_row = fft_1D(signalRow[y], dir);

		image_rows_fft[y] = one_row;
		r++;

	}
	vector<vector<complex<float>>> image_cols_fft(size);

	int x1 = 0;
	int sizeR = image_rows_fft.size();

	for(int x = 0; x < size; x++) {
		
		vector<complex<float>> one_col(size);

		for (int y = 0; y < size; y++) {

			one_col[y] = image_rows_fft[y][x];
		
		}
		
		image_cols_fft[x] = fft_1D(one_col, dir);

	}

	for (int o1 = 0; o1 < size; o1++) {
		
		vector<complex<float>> o1_row(size);

		for (int o2 = 0; o2 < size; o2++) {
		
			o1_row[o2] = image_cols_fft[o2][o1];
		
		}

		fft_2D_output[o1] = o1_row;

	}

	return fft_2D_output;
}



