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
	vec3 lightColor = vec3(1.0, 1.0, 1.0);

	// ambient
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;

	// diffuse 
	vec3 nnormal = normalize(normal);
	vec3 lightDir = normalize(lightPos - fragPos);
	float angle = max(dot(nnormal, lightDir), 0.0);
	vec3 diffuse = angle * lightColor;

	// specular
	float specularStrength = 1.0;
	vec3 viewDir = normalize(cameraPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, nnormal);
	float specFactor = pow(max(dot(viewDir, reflectDir), 0.0), 1);
	vec3 specular = specularStrength * specFactor * lightColor;

	vec3 result = (ambient + diffuse + specular);

	fragColor = vec4(objectColor, 1.0) * vec4(result, 1.0);
}