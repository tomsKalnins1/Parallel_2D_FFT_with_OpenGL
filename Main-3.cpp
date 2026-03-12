#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <complex>
#include <thread>
#include <barrier>
#include <mutex>

//#define M_PI 3.14159265358979323846264338

using namespace std;



mutex m;

struct vec{

	float real;
	float imag;

	vec operator+(const vec &a){
		
		
		vec res{real + a.real, imag + a.imag};
		
		return res;
	}

	vec operator*(const vec &a){
		
		vec res{ real * a.real - imag * a.imag, real * a.imag + imag * a.real };

	
		
		return res;
	}

	vec operator-(const vec& a) {


		vec res{real - a.real, imag - a.imag };
		
		return res;
	}


};
/*
vector<complex<float>> fft_1D_iter(vector<complex<float>> input) {
	vector<complex<float>> output(input.size());
	int k = 2; //the num of elements i.e. N
	int num_lvls = log2(input.size());
//	int num_lvls_1 = log_2_fft(input.size());
	cout << "log2 from lib = " << num_lvls << '\n';

	cout << "OWN log2 result = " << num_lvls_1 << '\n';



	for (int l = num_lvls; l > 0; l--) {
	
		int num_blocks = 1 << (l - 1);

		//cout << "num_blocks = " << num_blocks << " with " << k << "  elements on each BLOCK     on level " << l << "  out of  : " << num_lvls << '\n';

		for (int block = 0; block < num_blocks; block++) {
		
			for (int i = 0; i < k/2; i++) {
			
				complex<float> twiddle = complex<float>(cos(2.0f * M_PI * i / k), -sin(2.0f * M_PI * i / k));

				complex<float> freq = input[block * k + i] + input[block * k + i + k / 2] * twiddle;
				complex<float> freq_alias = input[block * k + i] - input[block * k + i + k / 2] * twiddle;

				input[block * k + i] = freq;
				input[block * k + i + k / 2] = freq_alias;

				//cout << " EVEN input index  =  " << ( block * k + i ) << " = ( block ) " << block << " * ( k ) " << k << " + ( i ) " << i << '\n';
				//cout << " ODD input index  =  " << ( block * k + i + k / 2 ) << " = ( block ) " << block << " * ( k ) " << k << " + ( i ) " << i << " + ( k/2 ) " << k/2 << '\n';

			}

		
		
		}

		k *= 2;
	}


	return input;
}

*/

typedef struct { float real; float imag;} cmplx;

struct complex_1 {

	float real;
	float imag;

};


unsigned int reverseBits(unsigned int num)
{
	unsigned int count = 31;
	unsigned int reverse_num = num;

	num >>= 1;
	while (num != 0)
	{
		reverse_num <<= 1;
		reverse_num |= num & 1;
		num >>= 1;
		count--;
	}
	reverse_num <<= count;
	return reverse_num;
}

barrier my_barrier{128};



vector<vec> input(256);

vector<vec> buffer(256);

vector<vec> ifft(256);


unsigned int rev(unsigned int n, unsigned int num_bits){

    unsigned int res = 0;

    unsigned int i = 0;

    while(i <= num_bits - 1){
    
        res <<= 1;

        if((n & 1) == 1){
            
            res |= 1;

        }

        n >>= 1;

        i++;

    }

    return res;
}

void permutate1(int t_id){
    
    unsigned int num = 1;
    unsigned int bits = 0; //set to zero or 1



    while(num < 256 ){
        
        num <<= 1;
        bits++;
    
    }
        //vec texCoor = ivec2(gl_GlobalInvocationID.xy);




        buffer[t_id] = input[rev(t_id, bits)];




	 
	my_barrier.arrive_and_wait();
	
	int pair = t_id + 128;

	buffer[pair] = input[rev(pair, bits)];

	my_barrier.arrive_and_wait();



}


void fft(int t_id){

  //  ivec2 texC_g = ivec2();



    unsigned int k = 2;
    unsigned int num_lvls = ( unsigned int )(log2(256));

    for(unsigned int lvl = 0; lvl < num_lvls; lvl++){
		vec v = { 0.0f, 0.0f };
		vec v_1 = { 0.0f, 0.0f };
   
            float angle = 2.0 * M_PI * float((t_id % (k/2))) / float(k);

            vec twiddle = {cos(angle), -sin(angle)};

            unsigned int block = t_id / (k / 2);
            unsigned int offset = t_id % (k / 2);

            unsigned int e = block * k + offset;
            unsigned int o = e + (k / 2);

		string index = "  id = " + to_string(t_id) + "  even = " + to_string(e) + "  odd = " + to_string(o)+ "  K   = " + to_string(k) + "  sampled freq =  " + to_string((t_id % (k / 2))) + 
			"  block = " + to_string(block) +  "  offset = " + to_string(offset);
	//	cout << index << '\n';

            vec even = input[e];

			my_barrier.arrive_and_wait();
            vec odd = input[o];
			my_barrier.arrive_and_wait();
            vec freq = even + (odd * twiddle);
            vec freq_alias = even - (odd * twiddle);

            v = freq;

            v_1 = freq_alias;

            buffer[e] = {v.real, v.imag};

            buffer[o] = {v_1.real, v_1.imag};

            
my_barrier.arrive_and_wait();


            input[o] = buffer[o];



            input[e] = buffer[e];



           my_barrier.arrive_and_wait();



        k *= 2;

          my_barrier.arrive_and_wait();
       
	}


     

  // synchronize();

   }


void print_buffer() {

	string buff = "";
	string line = "";
	for (int i = 0; i < buffer.size(); i++) {
		
		buff += " BUFFER real = " + to_string(buffer[i].real) + " imag = " + to_string(buffer[i].imag) + "\n";
		
	
	}

	cout << "BUFFER : \n" << buff << '\n';


}

void print_input() {

	string buff = "";
	string line = "";
	for (int i = 0; i < input.size(); i++) {

		buff += " INPUT real = " + to_string(input[i].real) + " imag = " + to_string(input[i].imag) + "\n";


	}

	cout << "INPUT : \n" << buff << '\n';


}

void print_ifft() {

	string buff = "";
	string line = "";
	for (int i = 0; i < ifft.size(); i++) {

		buff += " ifft real = " + to_string(ifft[i].real) + " imag = " + to_string(ifft[i].imag) + "\n";


	}

	cout << "ifft : \n" << buff << '\n';


}

void print_mag_input() {
	
	float mag = 0;
	float max_mag = 0;
	int freq = 0;

	for (int i = 0; i < input.size(); i++) {
		
		mag = sqrt(pow(input[i].real, 2.0f) + pow(input[i].imag, 2.0f));

		if (mag > max_mag && i != 0) {

			max_mag = mag;
			freq = i;

		}
	
	}

	cout << "MAX_MAG = " << max_mag/256.0f << " AT freq = \t" << freq << '\n';

}

void print_mag_buffer() {

	float mag = 0;
	float max_mag = 0;
	int freq = 0;

	for (int i = 0; i < buffer.size(); i++) {

		mag = sqrt(pow(buffer[i].real, 2.0f) + pow(buffer[i].imag, 2.0f));

		if (mag > max_mag && i != 0) {

			max_mag = mag;
			freq = i;

		}

	}

	cout << "MAX_MAG = " << max_mag/256.0f << " AT freq = \t" << freq << '\n';

}



int main() {

	vec v_1 = {2.0f, 2.0f};

	vec v_2 = {2.0f, 2.0f};

	vec res = (v_1 * v_2); 

	cout <<"mult real = " << res.real << " imag  = " << res.imag << '\n';

	cout << (v_1 + v_2).real << '\n' ;

    unsigned int num = 1;
    unsigned int bits = 0;



    while(num < 256 ){
        
        num <<= 1;
        bits++;
    
    }


	cout << "rev(7) = " <<(int) rev(255, bits) << " num bits = " << bits << '\n'; 

	vector<thread> t;

	
	for (int i = 0; i < 256; i++) {

		input[i] = { (float)sin(2.0f * M_PI * 61.0f * ((float)i) / 256.0f), 0.0f};

	}

//	print_input();

	for(int i = 0; i < 128; i++){
			thread t1(permutate1, i);
	
		   t.push_back(std::move(t1));
	
	}


	for(int i = 0; i < 128; i++){
	
		   t[i].join();
	
	}

	t.clear();



	for (int i = 0; i < 256; i++) {
	
		input[i] = buffer[i];

	}

//	print_input();
	
	for(int i = 0; i < 128; i++){
			thread t1(fft, i);
	
		   t.push_back(std::move(t1));
	
	}


	for(int i = 0; i < 128; i++){
	
		   t[i].join();
	
	}
	t.clear();

	print_mag_input();
	print_mag_buffer();



	for (int x = 0; x < 256; x++) {
	
	
		for (int i = 0; i < 256; i++) {
		
			
			ifft[x].real += input[i].real * cos(2.0f * M_PI * ((float)x * i) / 256.0f) - input[i].imag * sin(2.0f * M_PI * ((float)x * i) / 256.0f);

		}

		ifft[x].real /= 256.0f;
	}


//	print_ifft();


	return 0;
}
