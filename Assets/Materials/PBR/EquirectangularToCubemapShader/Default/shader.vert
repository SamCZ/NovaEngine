#version 330 core

in vec3 a_Position;

out vec3 WorldPos;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

void main() {
	WorldPos = a_Position;
	gl_Position = ProjectionMatrix * ViewMatrix * vec4(a_Position, 1.0);
}