#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gNoise;

uniform vec2 ScreenSize;

uniform vec3 Samples[64];

uniform mat4 PerspectiveMatrix;

int kernelSize = 64;
uniform float radius = 0.5;
uniform float bias = 0.025;

const float zNear = 0.1;
const float zFar = 1000.0;

const float MAX_DISTANCE = -0.5;

void main() {
	vec2 noiseScale = vec2(ScreenSize.x / 5.0, ScreenSize.y / 5.0);

	vec3 fragPos   = texture(gPosition, TexCoords).xyz;

	if (fragPos.z > MAX_DISTANCE) {
        FragColor = vec4(1.0);
        return;
    }

	vec3 normal = texture(gNormal, TexCoords).rgb;
    vec3 randomVec = texture(gNoise, TexCoords * noiseScale).xyz;
    randomVec = vec3(0.0);

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i) {
	    vec3 sample = TBN * Samples[i];
	    sample = fragPos + sample * radius; 
	   //vec3 sample = fragPos;

	    vec4 offset = vec4(sample, 1.0);
        offset = PerspectiveMatrix * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

		float sample_depth = texture(gPosition, offset.xy).z;
        float range_check = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sample_depth));
        occlusion += (sample_depth >= sample.z ? 1.0 : 0.0) * range_check;
	}
	occlusion = 1.0 - (occlusion / kernelSize);
	occlusion = clamp(occlusion, 0.0, 1.0);
	FragColor = vec4(texture(gAlbedoSpec, TexCoords).xyz * vec3(occlusion), 1.0);
	/*vec3 fragPos   = texture(gPosition, TexCoords).xyz;

	vec4 wpos = PerspectiveMatrix * vec4(fragPos, 1.0);
	vec2 texc = (wpos.xy / wpos.w) * 0.5 + 0.5;
	texc -= bias;

	float d = texture(gPosition, texc).z;

	FragColor.rgb = vec3(fragPos.z);
	FragColor.a = 1.0;*/

	//FragColor = texture(gAlbedoSpec, TexCoords);
}