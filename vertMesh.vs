
#version 330 core
layout (location = 0) in vec3 aPos; //in the state machine the input positions that are the raw coordinates at at location 0
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aTang;
layout (location = 4) in vec3 aBiTang;



out vec2 texCoor;
out vec3 Normal;
out vec3 FragPos;
out vec3 aTang;
out vec3 aBiTang;


uniform mat4 lookAt;
uniform mat4 rotate;
uniform mat4 translate;
uniform mat4 projection;
uniform mat4 scale;



void main(){
  
   gl_Position =  projection * lookAt * translate * rotate * scale * vec4(aPos, 1.0);
   FragPos = vec3(translate * rotate * scale * vec4(aPos, 1.0));
   Normal = mat3(transpose(inverse(rotate * scale))) * aNormal;
   texCoor = vec2(aTex.x, aTex.y);

}

