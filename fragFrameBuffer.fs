
#version 330 core

out vec4 FragColor;
//out vec aPos;



in vec2 texCoor; //the coordinates of vertices based on which the tecture is applied

uniform sampler2D filterTexture;
uniform sampler2D last;

void main(){

//!	vec3 color = vec3(texture(filterTexture, texCoor))*0.075;
//vec3 addT = texture(filterTexture, texCoor).rgb + texture(filterTexture, texCoor).rgb;
FragColor = texture(filterTexture, texCoor);
//FragColor = texture(filterTexture, texCoor);
//!	FragColor = vec4(color, 1.0);
}
