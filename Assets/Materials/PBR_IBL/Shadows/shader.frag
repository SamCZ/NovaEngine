#version 330 core

in vec2 TexCoords;
in vec3 WorldPos;
in vec4 ScreenPos;
in vec3 TriplanarCoords;
in vec3 Normal;
in vec4 ShadowCoords;
in float DistanceToVertex;

uniform sampler2D albedoMap;

uniform bool TriplanarTexturing;
uniform float TriplanarScale = 1.0;

uniform bool UseAlpha = true;
uniform float AphaThreshold = 0.5;


vec3 pow3(vec3 v, float e) {
    return vec3(pow(v.x, e), pow(v.y, e), pow(v.z, e));
}

vec2 maxv(float m, vec2 v) {
	return vec2(max(m, v.x), max(m, v.y));
}

vec3 getTriPlanarBlend(vec3 normal){
    vec3 blending = abs( normal );
    blending = normalize(max(blending, 0.00001)); // Force weights to sum to 1.0
    float b = (blending.x + blending.y + blending.z);
    blending /= vec3(b, b, b);

	/*vec3 blend = vec3(0.0);
	// Blend for sides only
	vec2 xzBlend = abs(normalize(normal.xz));
	blend.xz = maxv(0, xzBlend - 0.67);
	blend.xz /= dot(blend.xz, vec2(1,1));
	// Blend for top
	blend.y = clamp((abs(normal.y) - 0.675) * 80.0, 0.0, 1.0);
	blend.xz *= (1 - blend.y);*/

	/*vec3 blend = pow3(abs(normal.xyz), 4);
	blend /= dot(blend, vec3(1,1,1));*/

    return blending;
}

vec4 triplanarPass(sampler2D texture, float scale, float ts) {
    vec3 blendAxes = getTriPlanarBlend(Normal);
    //blendAxes = vec3(1.0);

    vec3 scaledWorldPos = TriplanarCoords.xyz / scale;

    vec4 xProjection = texture2D(texture, vec2(scaledWorldPos.y, scaledWorldPos.z) + ts) * blendAxes.x;
    vec4 yProjection = texture2D(texture, vec2(scaledWorldPos.x, scaledWorldPos.z) + ts) * blendAxes.y;
    vec4 zProjection = texture2D(texture, vec2(scaledWorldPos.x, scaledWorldPos.y) + ts) * blendAxes.z;
    
    vec4 color = xProjection + yProjection + zProjection;
    color.a /= 3.0;
    return color;

    /*vec3 normal = normalize(Normal);
    vec3 tpweights = abs(normal);
    tpweights = (tpweights - 0.2) * 7.0;
    tpweights = max(tpweights, vec3(0.0));
    tpweights /= tpweights.x + tpweights.y + tpweights.z;

    vec3 signs = sign(normal);

    vec2 tpcoord1 = vec2(vec2(-signs.x * TriplanarCoords.z, TriplanarCoords.y) * scale);
    vec2 tpcoord2 = vec2(vec2(signs.y * TriplanarCoords.x, -TriplanarCoords.z) * scale);
    vec2 tpcoord3 = vec2(vec2(signs.z * TriplanarCoords.x, TriplanarCoords.y) * scale);

    vec4 xProjection = texture2D(texture, tpcoord1) * tpweights.x;
    vec4 yProjection = texture2D(texture, tpcoord2) * tpweights.y;
    vec4 zProjection = texture2D(texture, tpcoord3) * tpweights.z;

    vec4 color = xProjection + yProjection + zProjection;
    color.a /= 3.0;*/
    return color;
}

vec4 triplanar(sampler2D texture) {
    return triplanarPass(texture, TriplanarScale, 1.0);
}

vec4 Tex2D(sampler2D smp, vec2 texCoord, bool trip) {
    if(trip) {
        //return triplanarPass(texture, TriplanarScale, 1.0);
        //float triBlend = smoothstep(changeDst - gradientSize, changeDst + gradientSize, DistanceToVertex);
        //return mix(triplanarPass(smp, TriplanarScale, 1.0), triplanarPass(smp, maxBlend, 1.0), triBlend);
		//return triplanarPass(smp, TriplanarScale, 1.0);
    }
    return texture2D(smp, texCoord);
}

vec4 Tex2D(sampler2D smp, vec2 texCoord) {
    if(TriplanarTexturing) {
        //return triplanarPass(texture, TriplanarScale, 1.0);
        //float triBlend = smoothstep(changeDst - gradientSize, changeDst + gradientSize, DistanceToVertex);
        //return mix(triplanarPass(smp, TriplanarScale, 1.0), triplanarPass(smp, maxBlend, 1.0), triBlend);
		//return triplanarPass(smp, TriplanarScale, 1.0);
    }
    return texture2D(smp, texCoord);
}

void main() {
    vec4 albedoFullColor = Tex2D(albedoMap, TexCoords);
    if(UseAlpha) {
        if(albedoFullColor.a < AphaThreshold) {
            discard;
        }
    }
}