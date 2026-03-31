
#version 330 core

#define M_PI 3.1415926535897932384626433832795

out vec4 FragColor;

in vec2 tex_coor;

uniform sampler2D real_imag;
uniform int wave_lim_0;
uniform int wave_lim_1;
uniform float N;

void main(){


	float waveSum = 0;
	int N_i = int(N);
	ivec2 coors = ivec2(tex_coor * (N - 1)); 

	vec4 freq = vec4(0.0, 0.0, 0.0, 0.0);
	float angle = 0;
	float magnitude = 0;
	float visualization = 0;

	freq = texelFetch(real_imag, ivec2((wave_lim_0),(wave_lim_1)), 0);
	angle  = 2.0 * M_PI * ((wave_lim_0) * coors.x / N + (wave_lim_1) * coors.y /N);
	magnitude = pow(pow(freq.y, 2.0) + pow(freq.x, 2.0), 0.5);
	visualization = log2(magnitude);
	
	waveSum += visualization*(freq.x * cos(angle) - freq.y * sin(angle));


	freq = texelFetch(real_imag, ivec2(( N_i/2 + wave_lim_0), (wave_lim_1)), 0);
	angle  = 2.0 * M_PI * ((N_i/2.0 + wave_lim_0) * coors.x / N + (wave_lim_1) * coors.y /N);
	magnitude = pow(pow(freq.y, 2.0) + pow(freq.x, 2.0), 0.5);
	visualization = log2(magnitude);

	waveSum += visualization*(freq.x * cos(angle) - freq.y * sin(angle));


	freq = texelFetch(real_imag, ivec2((wave_lim_0), ( N_i/2 + wave_lim_1)), 0);	
	angle  = 2.0 * M_PI * ((wave_lim_0) * coors.x / N + ( N_i/2 + wave_lim_1) * coors.y / N);
	magnitude = pow(pow(freq.y, 2.0) + pow(freq.x, 2.0), 0.5);
	visualization = log2(magnitude);

	waveSum += visualization*(freq.x * cos(angle) - freq.y * sin(angle));


	freq = texelFetch(real_imag, ivec2((N_i/2 + wave_lim_0), (N_i/2 + wave_lim_1)), 0);
	angle  = 2.0 * M_PI * ((N_i/2 + wave_lim_0) * coors.x / N + (N_i/2 + wave_lim_1) * coors.y /N);
	magnitude = pow(pow(freq.y, 2.0) + pow(freq.x, 2.0), 0.5);
	visualization = log2(magnitude);

	waveSum += visualization*(freq.x * cos(angle) - freq.y * sin(angle));


	float normalize = 1.0 / (4 * N);
	waveSum *= normalize;
	FragColor = vec4(waveSum, waveSum, waveSum, 1.0);

}
