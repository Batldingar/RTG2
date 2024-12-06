#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aColor;
layout (location = 4) in vec3 aOffset;

out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;
out vec3 vColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vColor = aColor; // Pass color to fragment shader
    texCoord = aUV;   
	normal	 = mat3(transpose(inverse(model))) * aNormal;
	vec3 finalDestination = aPosition + aOffset;
	fragPos  = vec3(model * vec4(finalDestination, 1.0));
    gl_Position = projection * view * model * vec4(finalDestination, 1.0);
}