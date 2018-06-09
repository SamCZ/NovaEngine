#version 330 core

in vec3 a_Position;
in vec2 a_TexCoord;

out vec2 TexCoords;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

void main() {
	vec4 worldPos = ModelMatrix * vec4(a_Position, 1.0);
	gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
	TexCoords = a_TexCoord;
}