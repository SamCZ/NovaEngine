#version 330 core

in vec3 a_Position;
in vec3 a_Normal;
in vec2 a_TexCoord;
in mat4 a_ModelMatrix;

out vec2 TexCoords;
out vec3 WorldPos;
out vec4 ScreenPos;
out vec3 TriplanarCoords;
out vec4 ShadowCoords;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ShadowMatrix;
uniform vec4 ClipPlane;
uniform vec3 ViewPos;
uniform vec2 TexOffset = vec2(0.0);
uniform bool UseInstancing;

uniform bool TriplanarWorld = true;

uniform vec2 TexScale = vec2(1.0, 1.0);

void main() {
	mat4 modelMatrix = a_ModelMatrix;
	if(!UseInstancing) {
		modelMatrix = ModelMatrix;
	}
	vec4 worldPos = modelMatrix * vec4(a_Position, 1.0);
	gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
	TexCoords = (a_TexCoord * TexScale) + TexOffset * 0.01;
	WorldPos = worldPos.xyz;
	ScreenPos = ViewMatrix * worldPos;
	if(TriplanarWorld) {
		TriplanarCoords = worldPos.xyz;
	} else {
		mat4 nullPos = modelMatrix;
		nullPos[3][0] = 0; // first entry of the fourth column.
		nullPos[3][1] = 0; // second entry of the fourth column.
		nullPos[3][2] = 0;
		nullPos[3][3] = 1;
		TriplanarCoords = (vec4(a_Position, 1.0) * nullPos).xyz;
	}

	ShadowCoords = ShadowMatrix * worldPos;
}