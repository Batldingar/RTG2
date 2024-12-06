#version 330 core

out vec4 fragColor;

in vec3 normal;
in vec2 texCoord;
in vec3 fragPos;


uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform sampler2D texture_diffuse1;

void main()
{   
	vec3 lightColor = vec3(1.0,1.0,1.0);

	// ambient
	float ambientFactor = 0.1f;
	vec3  ambient = ambientFactor * lightColor;

	// diffuse 
	vec3 nnormal = normalize(normal);

	vec3 lightDir = normalize(lightPos - fragPos);
	float diffFactor = max(dot(nnormal,lightDir),0.0f);
	vec3 diffuse = diffFactor * lightColor;

	// specular
	float specularStrength = 1.0;
	vec3 viewDir = normalize(cameraPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, nnormal); 
	float specFactor = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 specular = specularStrength * specFactor * lightColor; 
	
	
	// combine 
	vec3 result = objectColor * (ambient + diffuse + specular); 

    fragColor = vec4(result.rgb, 1.0);


}