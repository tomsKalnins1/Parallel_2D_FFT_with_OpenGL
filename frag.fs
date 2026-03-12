
#version 330 core

#define M_PI 3.1415926535897932384626433832795

out vec4 FragColor;

in vec2 TexCoor;



uniform sampler2D real_imag;

uniform int wave_lim;




void main(){


	float waveSum = 0;


	for(int x = 0; x < 256; x++){
	
	for(int y  = 0; y < 256; y++){
		//vec2 freq = texture(real_imag, TexCoor).rg;
		vec4 freq = texelFetch(real_imag, ivec2(x, y), 0);

	ivec2 coors = ivec2(TexCoor * 255.0); //LITERALLY THIS ONE LINE CAUSE ME TO SPEND MANY DAYS PRINTIGN VALUES AND DEBUGGIND, AND LOSING MY MIND ->
											//on mac os the floating point operations are done differently that on windows due to I guess CPU architecture
											//but what ever is the exact difference it is large enough to accumulate to insane level to make 2f IFFT output awful
		
	float angle  = 2.0 * M_PI * (x * coors.x / 256.0 + y * coors.y /256.0);

	waveSum += freq.x * cos(angle) - freq.y * sin(angle);

	
	}
	
	}

	float normalize = 1.0 / (256.0 * 256.0);

	waveSum *= normalize;



	FragColor = vec4(waveSum, waveSum, waveSum, 1.0);
	//FragColor = vec4(1.0, 0.0, 1.0, 1.0);
	//FragColor = texelFetch(real_imag, ivec2(TexCoor * 255), 0);





}
