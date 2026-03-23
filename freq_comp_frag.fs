
#version 330 core

#define M_PI 3.1415926535897932384626433832795

out vec4 FragColor;

in vec2 TexCoor;



uniform sampler2D real_imag;

uniform int wave_lim;


uniform float N;






void main(){


	float waveSum = 0;
	
	int N_i = int(N);

	vec4 freq = texelFetch(real_imag, ivec2((wave_lim),(wave_lim)), 0);

	ivec2 coors = ivec2(TexCoor * (N - 1)); 
		
	float angle  = 2.0 * M_PI * ((wave_lim) * coors.x / N + (wave_lim) * coors.y /N);

	waveSum += freq.x * cos(angle) - freq.y * sin(angle);


	freq = texelFetch(real_imag, ivec2(( N_i/2 + wave_lim), (wave_lim)), 0);


		
	angle  = 2.0 * M_PI * (( N_i/2 + wave_lim) * coors.x / N + (wave_lim) * coors.y /N);

	waveSum += freq.x * cos(angle) - freq.y * sin(angle);

	

	freq = texelFetch(real_imag, ivec2((wave_lim), ( N_i/2 + wave_lim)), 0);

		
	angle  = 2.0 * M_PI * ((wave_lim) * coors.x / N + ( N_i/2 + wave_lim) * coors.y / N);

	waveSum += freq.x * cos(angle) - freq.y * sin(angle);

	


	freq = texelFetch(real_imag, ivec2((N_i/2 + wave_lim), (N_i/2 + wave_lim)), 0);


		
	angle  = 2.0 * M_PI * ((N_i/2 + wave_lim) * coors.x / N + (N_i/2 + wave_lim) * coors.y /N);

	waveSum += freq.x * cos(angle) - freq.y * sin(angle);


	


	float normalize = 1.0 / (1.0 * N);

	waveSum *= normalize ;

	FragColor = vec4(waveSum, waveSum, waveSum, 1.0);







}
