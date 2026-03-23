
#version 330 core

out vec4 FragColor;
//out vec aPos;



in vec2 texCoor; //the coordinates of vertices based on which the tecture is applied


uniform sampler2D filterTexture;
uniform sampler2D last;

void main(){

FragColor = texture(filterTexture, texCoor);

}
