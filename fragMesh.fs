
#version 330 core

out vec4 FragColor;



//in vec3 vertexColor;
in vec2 texCoor; //the coordinates of vertices based on which the tecture is applied
in vec3 Normal;
in vec3 FragPos;
//in vec3 LightColorOut;
in vec3 aTang;
in vec3 aBiTang;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

struct Material{

	vec3 ambient;
	vec3 specular;
	vec3 diffuse;
	float shininess;

};
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_height1;

uniform Material material;

void main(){

	float ambientStrength = 0.1;
	vec3 ambient = material.ambient* lightColor;

	vec3 norm = normalize(Normal);
	vec3 direction = normalize(lightPos - FragPos);
	float diff = max(dot(norm, direction), 0.0);
	vec3 diffuse = (diff * material.diffuse)* lightColor;//id diff is 0 light and plane are looking in opposite direction => no light 
	

	float specularIntensity = 0.5;
	vec3 viewDir = normalize(viewPos - FragPos);//opengl linearly interpolates the FragPos it receives from the vertex shader
	vec3 reflectedLight = reflect(-direction, norm);//reflect function needs vector that points from light to the fragment => (-1)*direction

	float spec = pow(max(dot(reflectedLight, viewDir), 0.0), material.shininess);//the lasrget he power the quicker spec tends to 0 the sharperedges for reflection- > the more reflective the surface
	vec3 specular = (spec * material.specular) * lightColor;

	vec3 result = (ambient + diffuse + specular);



	//FragColor = vec4(result, 1.0);
	FragColor = texture(texture_diffuse1, texCoor);

}
