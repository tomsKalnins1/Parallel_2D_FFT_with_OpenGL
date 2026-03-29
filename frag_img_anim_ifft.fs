
#version 330 core

#define M_PI 3.1415926535897932384626433832795

out vec4 FragColor;

in vec2 tex_coor;



uniform sampler2D real_imag;

uniform int wave_lim_0;
uniform int wave_lim_1;

uniform float N;






void main(){


	float waveSum = 0.0;
	
	int N_i = int(N);

	for(int x = 0; x < wave_lim_0; x++){
	
		for(int y  = 0; y < wave_lim_1; y++){

		vec4 freq = texelFetch(real_imag, ivec2(x, y), 0);

		ivec2 coors = ivec2(tex_coor * (N - 1));
		
		float angle  = 2.0 * M_PI * (x * coors.x / N + y * coors.y /N);

		waveSum += freq.x * cos(angle) - freq.y * sin(angle);

	
		}
	
	}

	
	
	for(int x = N_i/2; x < N_i/2 + wave_lim_0; x++){
	
		for(int y  = 0; y < wave_lim_1; y++){

			vec4 freq = texelFetch(real_imag, ivec2(x, y), 0);
			ivec2 coors = ivec2(tex_coor * (N - 1.0));
			float angle  = 2.0 * M_PI * (x * coors.x / N + y * coors.y /N);
			waveSum += freq.x * cos(angle) - freq.y * sin(angle);

		}
	
	}

	for(int x = 0; x < wave_lim_0; x++){
	
		for(int y  = N_i/2 ; y < N_i/2 + wave_lim_1; y++){

			vec4 freq = texelFetch(real_imag, ivec2(x, y), 0);
			ivec2 coors = ivec2(tex_coor * (N - 1.0));
			float angle  = 2.0 * M_PI * (x * coors.x / N + y * coors.y / N);
			waveSum += freq.x * cos(angle) - freq.y * sin(angle);
	
		}
	
	}

	for(int x = N_i/2; x <N_i/2 + wave_lim_0; x++){
	
		for(int y  = N_i/2 ; y < N_i/2 + wave_lim_1; y++){

			vec4 freq = texelFetch(real_imag, ivec2(x, y), 0);
			ivec2 coors = ivec2(tex_coor * (N - 1.0));
			float angle  = 2.0 * M_PI * (x * coors.x / N + y * coors.y /N);
			waveSum += freq.x * cos(angle) - freq.y * sin(angle);

		}

	}
	
	float normalize = 1.0 / ( N * N);
	waveSum *= normalize;

	FragColor = vec4(waveSum, waveSum, waveSum, 1.0);

}
