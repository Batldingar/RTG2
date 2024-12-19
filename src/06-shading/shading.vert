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
uniform float time;

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

    // Calculate distance between vertex and mouse position in NDC space and move the vertex along z-axis based on time
    float frequencyBoost = 20; // changes wave length
    float vertexMouseDistance = distance(mousePos, fragPositionNDC);
    float speed = 2; // changes wave speed
    float timeBasedDistance = vertexMouseDistance * frequencyBoost - time * speed;
    float waveFriction = 75; // decreases wave amplitude with distance
    float amplitude = max(50 - vertexMouseDistance * waveFriction, 0); // changes wave height
    finalDestination.z += abs(sin(timeBasedDistance) * amplitude);

    // Actual final vertex position
    gl_Position = projection * view * model * vec4(finalDestination, 1.0);
}