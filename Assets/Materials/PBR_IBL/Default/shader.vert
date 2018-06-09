#version 330 core

in vec3 a_Position;
in vec3 a_Normal;
in vec2 a_TexCoord;

#ifdef USE_INSTANCING
in mat4 a_ModelMatrix;
#endif

out vec2 TexCoords;
out vec3 WorldPos;
out vec4 ScreenPos;
out vec3 TriplanarCoords;
out vec3 Normal;
out vec3 ViewNormal;
out vec4 ShadowCoords;
out float DistanceToVertex;
out vec3 a_FragPos;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ShadowMatrix;
uniform vec4 ClipPlane;
uniform vec3 ViewPos;
uniform vec2 TexOffset = vec2(0.0);
uniform bool UseInstancing = false;

uniform mat3 NormalMatrix;
uniform mat3 WorldNormalMatrix;

uniform bool TriplanarWorld = true;

uniform vec2 TexScale = vec2(1.0, 1.0);

void main() {
#ifdef USE_INSTANCING
	mat4 modelMatrix = a_ModelMatrix;
#else
	mat4 modelMatrix = ModelMatrix;
#endif
	vec4 worldPos = modelMatrix * vec4(a_Position, 1.0);
	gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
	TexCoords = (a_TexCoord * TexScale) + TexOffset * 0.01;
	WorldPos = worldPos.xyz;
	a_FragPos = (ViewMatrix * worldPos).xyz;
	ScreenPos = ViewMatrix * worldPos;
	//mat4 nMat = ViewMatrix * transpose(inverse(modelMatrix));
	//ViewNormal = normalize((ViewMatrix * transpose(inverse(modelMatrix)) * vec4(a_Normal, 0.0)).xyz);
	//ViewNormal = normalize(transpose(inverse(mat3(ViewMatrix) * mat3(modelMatrix))) * a_Normal);
	mat3 normalMatrix = mat3(transpose(inverse(ViewMatrix * modelMatrix)));
	ViewNormal = normalize(normalMatrix * a_Normal);
	Normal = normalize(WorldNormalMatrix * a_Normal);
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

	gl_ClipDistance[0] = dot(worldPos, ClipPlane);

	ShadowCoords = ShadowMatrix * worldPos;
	DistanceToVertex = length(WorldPos - ViewPos);
}