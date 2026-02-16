
#version 330 core

#define M_PI 3.1415926535897932384626433832795

out vec4 FragColor;

in vec2 TexCoor;

struct Signal{

	float phase;
	float amplitude;
	float frequency;

};

uniform sampler2D image;
uniform mat4 rotate;
uniform Signal signal;
//uniform bool;
uniform sampler2D last;

uniform sampler2D freq4;
uniform sampler2D freq5;
uniform sampler2D freq1;


void main(){


	vec4 TexCoorRot = rotate * vec4(TexCoor.x, TexCoor.y, 0.0, 1.0);

	vec2 rotTexCoors = vec2(TexCoorRot.x, TexCoorRot.y);
	
	//added 1 and divided by 2 to shift the wave up, wile max. val.  = 1 but min 0 =>  is this the reight way? 
	vec3 newTex = vec3(((signal.amplitude * cos(signal.frequency * 2.0 * M_PI  * rotTexCoors.x)) + 1.0)/2,
					((signal.amplitude * cos(signal.frequency * 2.0 * M_PI  * rotTexCoors.x)) + 1.0)/2,
					((signal.amplitude * cos(signal.frequency * 2.0 * M_PI  * rotTexCoors.x)) + 1.0)/2) + texture(last, TexCoor).rgb;

	vec3 sumOfTex = vec3(texture(freq4, TexCoor)) + vec3(texture(freq5, TexCoor)) + vec3(texture(freq1, TexCoor)) ;

	//FragColor = vec4(sumOfTex, 1.0);

	//FragColor = vec4(newTex, 1.0);
	FragColor = vec4((signal.amplitude * cos(signal.frequency * 2.0 * M_PI  * rotTexCoors.x + signal.phase) + signal.amplitude)/2,
					(signal.amplitude * cos(signal.frequency * 2.0 * M_PI * rotTexCoors.x + signal.phase) + signal.amplitude)/2,
					(signal.amplitude * cos(signal.frequency * 2.0 * M_PI * rotTexCoors.x + signal.phase) + signal.amplitude)/2, 1.0);

}
