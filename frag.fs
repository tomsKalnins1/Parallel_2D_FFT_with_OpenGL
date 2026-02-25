
#version 330 core

#define M_PI 3.1415926535897932384626433832795

out vec4 FragColor;

in vec2 TexCoor;


uniform vec2 freq_re_im[169];

uniform vec2 freq_re_im_1[169];

uniform vec2 freq_re_im_2[169];

uniform vec2 freq_re_im_3[169];

uniform vec2 freq_re_im_4[169];

uniform vec2 freq_re_im_5[169];





//uniform sampler2D frq_x_y_re_im;
//uniform sampler2D frq_phase_mag;



void main(){


	float waveSum = 0;

//	for(int u = 0; u < 256; u++){

		//for(int v = 0; v < 256; v++){
		
		//	vec4 freq_complex = texelFetch(frq_x_y_re_im, ivec2(u, v), 0);
		//	vec4 phase_mag = texelFetch(frq_phase_mag, ivec2(u, v), 0);

		//	vec4 freq_complex = vec4(0.5, 0.5, 0.5, 0.5);
		//	vec2 phase_mag = vec2(0.0, 0.0);

		//	waveSum += freq_complex.z * cos(2.0 * M_PI * (freq_complex.x * TexCoor.x + freq_complex.y * (1.0 - (1.0 - TexCoor.y)))) / (256.0 * 256.0 * 256.0);

		//	waveSum +=  (freq_complex.z * cos(2.0 * M_PI * (freq_complex.x * TexCoor.x + freq_complex.y * (1.0 - (1.0 - TexCoor.y))) + phase_mag.x) + freq_complex.w * sin(2.0 * M_PI * (freq_complex.x * TexCoor.x + freq_complex.y * (1.0 - (1.0 - TexCoor.y))) + phase_mag.x))* 2 /(256.0 * 256 * 256);

//		}

//	}
//	vec4 freq_complex = texelFetch(frq_x_y_re_im, ivec2(1, 1), 0);
//	vec4 phase_mag = texelFetch(frq_phase_mag, ivec2(1, 0), 0);
	//waveSum = freq_complex.z * cos(2.0 * M_PI * (freq_complex.x * TexCoor.x + freq_complex.y * (1.0 - (1.0 - TexCoor.y)))) / (256.0 * 256.0 * 256.0);
	int w = 0;
		for(int x = 0; x < 13; x++){
			for(int y = 0; y < 13; y++){
			float angle  = 2.0 * M_PI * (x * TexCoor.x + y * (TexCoor.y)) * 255.0/256.0;
			
			
	
			waveSum += (freq_re_im[w].x * cos(angle) - freq_re_im[w].y * sin(angle));
	
				w++;
				}
			}
			w= 0;

	for(int x = 13; x < 26; x++){
		for(int y = 13; y < 26; y++){
				float angle  = 2.0 * M_PI * (x * TexCoor.x + y * (TexCoor.y)) * 255.0/256.0;

		

			waveSum += (freq_re_im_1[w].x * cos(angle) - freq_re_im_1[w].y * sin(angle));

			w++;
		}
	}
	w = 0;
	
	for(int x = 26; x < 39; x++){
		for(int y = 26; y < 39; y++){
				float angle  = 2.0 * M_PI * (x * TexCoor.x + y * (TexCoor.y)) * 255.0/256.0;

			

			waveSum += (freq_re_im_2[w].x * cos(angle) - freq_re_im_2[w].y * sin(angle));
			w++;
	}

	}
	w = 0;
		for(int x = 243; x < 256; x++){
		for(int y = 0; y < 13; y++){
			float angle  = 2.0 * M_PI * (x * TexCoor.x + y *  (TexCoor.y)) * 255.0/256.0;

		

			waveSum += (freq_re_im_3[w].x * cos(angle) - freq_re_im_3[w].y * sin(angle));
			w++;
	}

	}

	w = 0;
		for(int x = 0; x < 13; x++){
		for(int y = 243; y < 256; y++){
				float angle  = 2.0 * M_PI * (x * TexCoor.x + y * ( TexCoor.y)) * 255.0/256.0;

		

			waveSum += (freq_re_im_4[w].x * cos(angle) - freq_re_im_4[w].y * sin(angle));
			w++;
	}

	}
	w = 0;
		for(int x = 243; x < 256; x++){
		for(int y = 243; y < 256; y++){
		

				float angle  = 2.0 * M_PI * (x * TexCoor.x + y *  (1.0 - TexCoor.y) ) * 255.0/256;

			waveSum += (freq_re_im_5[w].x * cos(angle) - freq_re_im_5[w].y * sin(angle));
			w++;
	}

	}

	float normalize = 1.0 / (256.0 * 256.0);

	waveSum *= normalize;


	//float angle_1  = 2.0 * M_PI * (freq_re_im_1[0].x * TexCoor.x * 255.0 / 256.0 + freq_re_im_1[0].y * (TexCoor.y * 255.0) / 256.0);

	//waveSum = mag_1[0]*cos(angle_1 + phase[0])/(256.0 * 256.0);
	FragColor = vec4(waveSum, waveSum, waveSum, 1.0);

//	FragColor = vec4(0.5, 0.1, 0.1, 1.0);


}
