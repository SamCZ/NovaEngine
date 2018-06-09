#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec2 TexCoordG[];

out vec2 TexCoords;
out vec2 MapTexCoords;
out vec3 WorldPos;
out vec4 WorldPosW;
out vec3 TriplanarCoords;
out vec3 Normal;
out float DistanceToVertex;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

uniform vec3 ViewPos;
uniform float ScaleY = 0.0;

uniform float TexScale = 1.0;

vec3 calcNormal(vec4 dp1, vec4 dp2, vec4 dp3) {
	vec3 v0 = gl_in[0].gl_Position.xyz + dp1.xyz;
	vec3 v1 = gl_in[1].gl_Position.xyz + dp2.xyz;
	vec3 v2 = gl_in[2].gl_Position.xyz + dp3.xyz;

	// edges of the face/triangle
    vec3 U = v1 - v0;
    vec3 V = v2 - v0;
    
    float nx = (U.y * V.z) - (U.z * V.y);
    float ny = (U.z * V.x) - (U.x * V.z);
    float nz = (U.x * V.y) - (U.y * V.x);
    
    /*vec2 uv0 = TexCoordG[0];
	vec2 uv1 = TexCoordG[1];
	vec2 uv2 = TexCoordG[2];

    vec2 deltaUV1 = uv1 - uv0;
	vec2 deltaUV2 = uv2 - uv0;
	
	float r = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
	
	Tangent = normalize((U * deltaUV2.y - V * deltaUV1.y)*r);
	BiTangent = normalize((V * deltaUV1.x - U * deltaUV2.x) * r);*/

    return normalize(vec3(nx, ny, nz));
}

void main() {
	vec3 normal = calcNormal(vec4(0,0,0,0), vec4(0,0,0,0), vec4(0,0,0,0));

	mat4 MVP = ProjectionMatrix * ViewMatrix;
	for (int i = 0; i < gl_in.length(); ++i) {
		vec4 vertexPos = gl_in[i].gl_Position;

		gl_Position = MVP * vertexPos;
		TexCoords = TexCoordG[i] * TexScale;
		MapTexCoords = TexCoordG[i];
		WorldPosW = gl_Position;
		WorldPos = vertexPos.xyz;
		TriplanarCoords = WorldPos;
		Normal = normal;
		DistanceToVertex = length(WorldPos - ViewPos);

		EmitVertex();
	}
	EndPrimitive();
}