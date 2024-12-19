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
uniform vec2 mousePos;

void main()
{
	vColor = aColor; // Pass color to fragment shader
    texCoord = aUV;   
	normal	 = mat3(transpose(inverse(model))) * aNormal;

	// Vertex position calculation in world space
	vec3 finalDestination = aPosition + aOffset;
	fragPos  = vec3(model * vec4(finalDestination, 1.0));

	// Transform to clip space...
    vec4 clipSpacePosition = projection * view * model * vec4(finalDestination, 1.0);

	// ...and to normalized device coordinates so that it matches the mouse pos space
    vec2 fragPositionNDC = clipSpacePosition.xy / clipSpacePosition.w;

    // Check distance to mouse position in NDC space
    if (distance(mousePos, fragPositionNDC) < 0.05) { // Adjust threshold as needed
        finalDestination.z += 100; // Move the vertex slightly along z-axis
    }

    // Actual final vertex position
    gl_Position = projection * view * model * vec4(finalDestination, 1.0);
}