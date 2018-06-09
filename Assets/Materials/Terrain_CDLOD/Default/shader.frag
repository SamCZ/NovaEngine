#version 330 core

struct DirLight {
    vec3 Direction;
    vec3 Position;
    vec3 Color;
};

struct PointLight {
    vec3 Position;
    vec3 Color;
    float Radius;
};

out vec4 FragColor;

in vec2 TexCoords;
in vec2 MapTexCoords;
in vec3 WorldPos;
in vec4 WorldPosW;
in vec3 TriplanarCoords;
in vec3 Normal;
in vec4 ShadowCoords;
in float DistanceToVertex;


uniform float ScaleY = 0.0;
uniform float HeightMapScale;
uniform sampler2D HeightMap;
uniform sampler2D HeightMapNormal;
uniform vec3 ViewPos;
uniform sampler2D ErosionMap;

uniform sampler2D ShadowMap;
uniform bool UseShadows;

uniform DirLight DirLights[16];
uniform int DirLightSize;

uniform PointLight PointLights[64];
uniform int PointLightCount;

uniform vec4 AmbientLight = vec4(0.0);

uniform sampler2D GrassMap;
uniform sampler2D GrassNormalMap;
uniform sampler2D RockMap;
uniform sampler2D RockNormalMap;
uniform sampler2D RockErosionMap;
uniform sampler2D RockErosionNormalMap;

vec3 getNormalFromMap(sampler2D normMap, vec2 texCoord) {
    vec3 nn = 2.0 * (texture(HeightMapNormal, MapTexCoords.st).rgb) - 1.0;
    vec3 tangentNormal = texture(normMap, texCoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(nn);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

const float near_plane = 0.1;
const float far_plane = 1000;

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

float ShadowCalculation() {
    float visibility = 1.0;
    //float bias = 0.005*tan(acos(dot(normal, -DirLights[0].Direction))); // cosTheta is dot( n,l ), clamped between 0 and 1
    //bias = clamp(bias, 0,0.01);
    float bias = 1.0 / (2048.0 * 1.0);
    if (texture( ShadowMap, ShadowCoords.xy ).z < ShadowCoords.z){
        visibility = 0.5;
    }

    return texture( ShadowMap, ShadowCoords.xy ).z;
}

void main() {
    float height = texture(HeightMap, MapTexCoords.st).r;
    vec3 erosion = texture(ErosionMap, vec2(MapTexCoords.s, MapTexCoords.t)).rgb;
    float erosionValue = (erosion.r + erosion.g + erosion.b) / 3.0;
    vec3 terrainNormal = normalize(2.0 * (texture(HeightMapNormal, MapTexCoords.st).rgb) - 1.0);


    vec3 grassNormal = getNormalFromMap(GrassNormalMap, TexCoords.st);
    vec3 rockNormal = getNormalFromMap(RockNormalMap, TexCoords.st);
    vec3 rockErosionNormal = getNormalFromMap(RockErosionNormalMap, TexCoords.st);

    vec3 totalNormal = vec3(0.0);
    totalNormal = mix(grassNormal, rockNormal, 1.0 - smoothstep(1.0, 0.0, height));
    totalNormal = mix(grassNormal, mix(totalNormal, rockErosionNormal, smoothstep(1.0, 0.8, erosionValue)), clamp(pow(height, 0.5), 0.0, 1.0));
    totalNormal = normalize(totalNormal);

    float depth = WorldPosW.z / WorldPosW.w;

    vec3 fixedUp = vec3(0.0, 1.0, 0.0);
    float up = dot(terrainNormal, fixedUp);
    float colBlend = 1.0 - smoothstep(0.8, 1.0, up);



    vec3 grassColor = vec3(0.2, 0.8, 0.2);
    grassColor = texture(GrassMap, TexCoords.st).rgb;

    vec3 rockErosionColor = vec3(0.4);
    rockErosionColor = texture(RockErosionMap, TexCoords.st).rgb;

    vec3 rockColor = vec3(0.8);
    rockColor = texture(RockMap, TexCoords.st).rgb;

    if(depth >= 0.999) {
        grassColor = texture(GrassMap, TexCoords.st / 50.0).rgb;
        rockErosionColor = texture(RockErosionMap, TexCoords.st / 1.0).rgb;
        rockColor = texture(RockMap, TexCoords.st / 50.0).rgb;
        //grassColor = vec3(0.0);
    }


    grassColor = mix(grassColor, grassColor * 0.01, clamp(pow(height, 0.5), 0.0, 1.0));

    vec3 finalColor = vec3(0.0);

    finalColor = mix(grassColor, rockColor, 1.0 - smoothstep(1.0, 0.0, height));
    finalColor = mix(grassColor, mix(finalColor, rockErosionColor, smoothstep(1.0, 0.8, erosionValue)), clamp(pow(height, 0.5), 0.0, 1.0));

    float brightness = dot(totalNormal, DirLights[0].Direction);
    vec3 diffuse = brightness * DirLights[0].Color;
    diffuse += AmbientLight.rgb;
    diffuse = clamp(diffuse, 0.1, 1.0);

    FragColor.rgb = (finalColor * diffuse) * 2.0;

    if(UseShadows) {
        FragColor.rgb *= ShadowCalculation();
    }
}