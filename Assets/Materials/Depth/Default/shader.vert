#version 330 core

in vec3 a_Position;
out vec4 FragPos;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

void main() {
	vec4 worldPos = ModelMatrix * vec4(a_Position, 1.0);
	FragPos = worldPos;
	gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
}