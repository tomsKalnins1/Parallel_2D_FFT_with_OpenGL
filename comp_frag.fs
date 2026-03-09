
#version 430 core

#define M_PI 3.1415926535897932384626433832795

out vec4 FragColor;

in vec2 TexCoor;



uniform sampler2D screen;






void main(){

vec3 col = texture(screen, TexCoor).rgb;

	FragColor = vec4(col, 1.0);

}
