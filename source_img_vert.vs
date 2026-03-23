#version 330 core
layout (location = 0) in vec2 aPos; //in the state machine the input positions that are the raw coordinates at at location 0

layout (location = 1) in vec2 aTex;


out vec2 texCoor;
uniform mat4 move;


void main(){

   gl_Position = move * vec4(aPos.x, aPos.y, 0.0, 1.0); 

   texCoor = vec2(aTex.x, aTex.y);

}
