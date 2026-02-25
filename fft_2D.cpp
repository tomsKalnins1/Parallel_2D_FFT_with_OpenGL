#include "fft_2D_images.h"




vector<complex<float>> fft2D::fft_1D(vector<complex<float>> signal) {

	int len = signal.size();

	//cout << "len = \t" << len << '\n';

	if (len <= 1) {

		return signal;

	}
	//std::cout << "len = " << len << '\n';
	vector<complex<float>> oddTerms(len/2);
	vector<complex<float>> evenTerms(len/2);

	
	vector<complex<float>> odd(len / 2);
	vector<complex<float>> even(len / 2);
	
	int n = 0;
	std::string oddStr = "";
	std::string evenStr = "";
	for (int i = 1; i < len; i += 2) {
	
		odd[n] = signal[i];
		oddStr += "( " + std::to_string(signal[i].real()) + ",  " + std::to_string(signal[i].imag()) + " )";

		n++;

	}

	n = 0;

	for (int k = 0; k < len; k += 2) {

		even[n] = signal[k];
		evenStr += "( " + std::to_string(signal[k].real()) + ",  " + std::to_string(signal[k].imag()) + " )";


		n++;
	}

	oddTerms = fft_1D(odd);
	evenTerms = fft_1D(even);


	vector<complex<float>> components(len);
	int m = 0;
	for (int j = 0; j < len/2; j++) {

		complex<float> twiddle = std::complex<float>(cos(M_PI * 2.0f * j / len), (-1) * sin(M_PI * 2.0f  * j/ len));



		components[j] = evenTerms[j] + twiddle * oddTerms[j];
		components[j + len / 2] = evenTerms[j] - twiddle * oddTerms[j];
		m = j;

	}

	

	return components;
}

vector<vector<complex<float>>> fft2D::fft_2D(vector<vector<complex<float>>> signalRow) {
	cout << "2D FFT CALLED !" << endl;
	int size = signalRow.size();
	vector<vector<complex<float>>> image_rows_fft(size);
	vector<vector<complex<float>>> fft_2D_output(size);

	int r = 0;
	for (int y = 0; y < size; y++) {
	//	cout << "beefore fft2d" << endl;
		vector<complex<float>> one_row = fft_1D(signalRow[y]);
//		cout << "ROW nr. " << y << endl;
		printMaxMag(one_row);

		image_rows_fft[y] = one_row;
	//	cout << "FFT FOR ROW : " << r << " COMPLETED " << endl;

		r++;
	}
	vector<vector<complex<float>>> image_cols_fft(size);
	cout << "FFT FOR ROW : " << r << " COMPLETED " << endl;

	int x1 = 0;
	int sizeR = image_rows_fft.size();

	cout << "NUM OF IMAGE ROWS FFT =  " << image_rows_fft.size() << endl;

	for(int x = 0; x < size; x++) {
		
		vector<complex<float>> one_col(size);

		for (int y = 0; y < size; y++) {

			one_col[y] = image_rows_fft[y][x];
		
		}
		
		image_cols_fft[x] = fft_1D(one_col);

	}

	cout << "NUM OF IMAGE COLS FFT =  " << image_cols_fft.size() << endl;

	for (int o1 = 0; o1 < size; o1++) {
		
		vector<complex<float>> o1_row(size);

		for (int o2 = 0; o2 < size; o2++) {
		
			o1_row[o2] = image_cols_fft[o2][o1];
		
		}

		fft_2D_output[o1] = o1_row;
	//	cout << "SIZE OF 2F FFT OUTPUT  " << fft_2D_output.size() << endl;

	}
	//cout << "SIZE OF 2F FFT OUTPUT  " << fft_2D_output.size() << endl;
	return fft_2D_output;
}

void fft2D::printMaxMag(vector<vector<complex<float>>> fft_2D_output) {

	int freqNr_X = 0;
	int freqNr_Y = 0;
	float maxMag = 0.0f;
	float mag = 0.0f;

	int size = fft_2D_output.size();

	std::string all_transforms = "";

	for (int x = 0; x < size; x++) {
	
		std::string row_1 = "";

		for (int y = 0; y < size; y++) {
		
			float re = fft_2D_output[x][y].real();
			float im = fft_2D_output[x][y].imag();
		//	row_1 += "( " + std::to_string((int) re) + ",  " + std::to_string((int) im) + " ), ";
			mag = sqrt(pow(re, 2.0f) + pow(im, 2.0f));
		//	cout << "MAG of row/FREQ X  = " << x << " ,  colFREQ Y =  " << y << "  =  " << mag/(size * size) << endl;
 
			

			if (mag > maxMag && (x || y ) ) {

				freqNr_X = x;
				freqNr_Y = y;
				maxMag = mag;



			}
		
		}

		all_transforms += row_1 + "END OF ROW \n";
	
	}
	//carelessly fucked the var names
	cout << "FREQ_NR_Y = " << freqNr_X << " FREQ_NR_X= " << freqNr_Y << " MAX_MAG = " << maxMag << endl;

	//cout << "ALL TRANSFORM ROWS : \n" << all_transforms;

}

void fft2D::printMaxMag(vector<complex<float>> fft_2D_output) {


	int freq = 0;
	float mag = 0;
	float maxMag = 0;
	float phase = 0;

	int size = fft_2D_output.size();

	for (int i = 0; i < size; i++) {
		
		float im = fft_2D_output[i].imag();
		float re = fft_2D_output[i].real();

		mag = sqrt(pow(im, 2.0f) + pow(re, 2.0f));

	//	cout << "MAG of freq nr. " << i << "  =  " << mag/(size/2) << endl;



		if (mag > maxMag && i != 0) {

			phase = atan(im / re);
			maxMag = mag;
			freq = i;

		}
	}

	//cout << "LARGEST MAGNITUDE  = " << maxMag << " FOR THE FREQ.  : " << freq <<  "  AND PHASE (deg) " << (phase * 180.0f/M_PI) << " SIZE OF FFT OUTPUT = " << fft_2D_output.size() << endl;
}