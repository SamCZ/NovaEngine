#version 330 core

struct DirLight {
    vec3 Direction;
    vec3 Position;
    vec3 Color;
    float Intensity;
};

struct PointLight {
    vec3 Position;
    vec3 Color;
    float Radius;
    bool HasShadowMap;
    samplerCube ShadowMap;
};

//out vec4 FragColor;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec3 FragNorm;
layout(location = 2) out vec3 FragPos;

in vec2 TexCoords;
in vec3 WorldPos;
in vec4 ScreenPos;
in vec3 TriplanarCoords;
in vec3 Normal;
in vec4 ShadowCoords;
in float DistanceToVertex;
in vec3 a_FragPos;
in vec3 ViewNormal;
/*uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;*/

#ifdef USE_ALBEDO_TEXTURE
uniform sampler2D albedoMap;
#else
uniform vec4 albedoColor;
#endif

#ifdef USE_NORMAL_TEXTURE
uniform sampler2D normalMap;
#endif
uniform bool invertNormalMap = false;
uniform bool invertNormalMapY = false;

#ifdef USE_NORMAL_PARAM
uniform vec3 normalVar;
uniform bool normalVarWorld;
#endif

#ifdef USE_PARALAX_MAP
uniform sampler2D paralaxMap;
uniform float paralaxScale = 0.0;
#endif

#ifdef USE_METALIC_TEXTURE
uniform sampler2D metallicMap;
#endif
uniform float metalicValue = 1.0;

#ifdef USE_ROUGHNESS_TEXTURE
uniform sampler2D roughnessMap;
#endif
uniform float roughnessValue = 1.0;

#ifdef USE_ROUGHNESS_SUBVAL
uniform float roughnessSub = 0.0;
#endif

#ifdef USE_AO_TEXTURE
uniform sampler2D aoMap;
#endif
uniform float aoValue = 1.0;

// IBL
uniform samplerCube IrradianceMap;
uniform samplerCube PrefilterMap;
uniform sampler2D BrdfLUT;

uniform sampler2D Bloom;
uniform bool UseBloom = false;
uniform bool UseBloomTriplanar = false;

uniform bool FlipYCoord = false;

#ifdef USE_TRIPLANAR
uniform float TriplanarScale = 1.0;
#endif

uniform bool UseAlpha = true;
uniform float AphaThreshold = 0.5;

uniform vec3 ViewPos;

const float PI = 3.14159265359;

uniform DirLight DirLights[16];
uniform int DirLightSize;

uniform PointLight PointLights[64];
uniform int PointLightCount;

uniform vec4 AmbientLight = vec4(0.0);

uniform mat4 ViewMatrix;
uniform mat4 lightProjection;
uniform float frustumSize = 1.0;

vec3 pow3(vec3 v, float e) {
    return vec3(pow(v.x, e), pow(v.y, e), pow(v.z, e));
}

#ifdef USE_TRIPLANAR
uniform float minBlend = 1.0;
uniform float maxBlend = 0.05;

uniform float changeDst = 100.0;
uniform float gradientSize = 50.0;

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
		return triplanarPass(smp, TriplanarScale, 1.0);
    }
    if(FlipYCoord) {
        texCoord = vec2(texCoord.s, 1.0 - texCoord.t);
    }
    return texture2D(smp, texCoord);
}
#endif

#ifdef USE_PARALAX_MAP
vec2 ParallaxMapping2(vec2 texCoords, vec3 viewDir)
{ 
    // number of depth layers
    const float minLayers = 1;
    const float maxLayers = 10;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xz * paralaxScale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(paralaxMap, currentTexCoords).r;
    int aa = 0;
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(paralaxMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;
        aa++;
        if(aa > 1000) break;
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(paralaxMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}
#endif

vec4 Tex2D(sampler2D smp, vec2 texCoord) {
#ifdef USE_TRIPLANAR
    //return triplanarPass(texture, TriplanarScale, 1.0);
    //float triBlend = smoothstep(changeDst - gradientSize, changeDst + gradientSize, DistanceToVertex);
    //return mix(triplanarPass(smp, TriplanarScale, 1.0), triplanarPass(smp, maxBlend, 1.0), triBlend);
    return triplanarPass(smp, TriplanarScale, 1.0);
#else
#ifdef USE_PARALAX_MAP
    if(abs(distance(ViewPos, WorldPos)) < 2.5) {
        texCoord = ParallaxMapping2(texCoord, normalize(ViewPos - WorldPos));
    }
#endif
    if(FlipYCoord) {
        texCoord = vec2(texCoord.s, 1.0 - texCoord.t);
    }
    return texture2D(smp, texCoord);
#endif
}

vec3 getNormalFromMap() {
#ifdef USE_NORMAL_PARAM
    if(normalVarWorld) {
        vec3 Q1  = dFdx(WorldPos);
        vec3 Q2  = dFdy(WorldPos);
        vec2 st1 = dFdx(TexCoords);
        vec2 st2 = dFdy(TexCoords);

        vec3 N   = normalize(Normal);
        vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
        vec3 B  = -normalize(cross(N, T));
        mat3 TBN = mat3(T, B, N);

        return normalize(TBN * normalVar);
    }
    return normalize(normalVar);
#elif defined(USE_NORMAL_TEXTURE)
    vec3 tangentNormal = Tex2D(normalMap, TexCoords).xyz;
    if(invertNormalMap) {
        tangentNormal.b = 1.0 - tangentNormal.b;
    }
    if(invertNormalMapY) {
        tangentNormal.r = 1.0 - tangentNormal.r;
    }
    tangentNormal = normalize(tangentNormal * 2.0 - 1.0);
    
    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
#else
    return Normal;
#endif
}

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   
// ----------------------------------------------------------------------------

const float zNear = 0.1;
const float zFar = 1000.0;

float LinearizeDepth(float z) {
    return (2.0 * zNear) / (zFar + zNear - z * (zFar - zNear)); 
}

const float MAX_REFLECTION_LOD = 5.0;

vec3 sampleOffsetDirections[20] = vec3[] (
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);  

float ShadowCalculation(vec3 fragPos, vec3 lightPos, samplerCube depthMap) {
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    /*float closestDepth = texture(depthMap, fragToLight).r;
    closestDepth *= 500.0;
    float currentDepth = length(fragToLight);
    float bias = 0.05; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;*/

    float shadow = 0.0;
    float bias   = 0.05;
    int samples  = 20;
    float far_plane = 500.0;
    float viewDistance = length(ViewPos - fragPos);
    //float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    float diskRadius = 0.005;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane;
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
    return shadow;
}

const float pointLightShadowMapBias = 0.00075;

float Depth(vec3 pos) {
    vec3 absPos = abs(pos);
    float z = -max(absPos.x, max(absPos.y, absPos.z));
    vec4 clip = lightProjection * vec4(0.0, 0.0, z, 1.0);
    return (clip.z / clip.w) * 0.5 + 0.5;
}

float PCSS_PointLight(vec3 lightPosition, samplerCube shadowCubeMap) {
    mat4 lightView = mat4(1,0,0,0, 
        0,1,0,0, 
        0,0,1,0, 
        -lightPosition.x,-lightPosition.y,-lightPosition.z, 1);

    mat4 invView = inverse(ViewMatrix);

    vec3 positionLightSpace = (lightView * invView * vec4(a_FragPos, 1)).xyz;
    float receiverDistance = Depth(positionLightSpace);
    
    float z = texture(shadowCubeMap, positionLightSpace).r;

    return (z < (receiverDistance - pointLightShadowMapBias)) ? 0 : 1;
}

void main() {
#ifdef USE_ALBEDO_TEXTURE
    vec4 albedoFullColor = Tex2D(albedoMap, TexCoords);
    vec3 albedo     = pow3(albedoFullColor.rgb, 2.2);
#else
    vec4 albedoFullColor = albedoColor;
    vec3 albedo     = albedoColor.rgb;
#endif

    //vec3 normal     = getNormalFromNormalMap();
    float metallic  = metalicValue;
#ifdef USE_METALIC_TEXTURE
    metallic *= Tex2D(metallicMap, TexCoords).r;
#endif
    float roughness = roughnessValue;
#ifdef USE_ROUGHNESS_TEXTURE
    roughness *= Tex2D(roughnessMap, TexCoords).r;
#endif
    float ao = aoValue;
#ifdef USE_AO_TEXTURE
    ao *= Tex2D(aoMap, TexCoords).r;
#endif
#ifdef USE_ROUGHNESS_SUBVAL
    roughness = roughnessSub - roughness;
#endif
    
    FragPos.xyz = a_FragPos;
    //FragPos.z = FragPos.z;
    //FragPos.z = LinearizeDepth(FragPos.z);
    //FragPos.w = zFar - LinearizeDepth(a_FragPos.z);
    

    vec3 N = getNormalFromMap();
    vec3 V = normalize(ViewPos - WorldPos);
    vec3 R = reflect(-V, N); 

    FragNorm = ViewNormal;

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);

    float shadowValue = 0.0;
    int numShadowsDepth = 0;

    for(int i = 0; i < PointLightCount; ++i) {
        // calculate per-light radiance
        vec3 L = normalize(PointLights[i].Position - WorldPos);
        vec3 H = normalize(V + L);
        float distance    = length(PointLights[i].Position - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = PointLights[i].Color * attenuation * PointLights[i].Radius;        
        
        float currentShadowValue = 1.0;
        if(PointLights[i].HasShadowMap) {
            currentShadowValue = 1.0 - ShadowCalculation(WorldPos, PointLights[i].Position, PointLights[i].ShadowMap);
            //currentShadowValue = PCSS_PointLight(PointLights[i].Position, PointLights[i].ShadowMap);
        }

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);        
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;     
        
        vec3 nominator    = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; 
        vec3 specular     = nominator / denominator;
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += ((kD * albedo / PI + specular) * radiance * NdotL) * (currentShadowValue); 
    }

    for(int i = 0; i < DirLightSize; ++i) {
        // calculate per-light radiance
        vec3 L = normalize(DirLights[i].Direction);
        vec3 H = normalize(V + L);
        vec3 radiance     = DirLights[i].Color * DirLights[i].Intensity;        
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);        
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        vec3 nominator    = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; 
        vec3 specular     = nominator / denominator;
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;     
    
    vec3 irradiance = texture(IrradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    vec3 prefilteredColor = textureLod(PrefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(BrdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * (AmbientLight.xyz + vec3(ao));
    
    float globalIntensity = 0.0;
    if(DirLightSize > 0) {
        globalIntensity = clamp(dot(vec3(0, 1, 0), DirLights[0].Direction), 0.0, 1.0);
    }
    vec3 color = Lo + (ambient * globalIntensity);

    // HDR tonemapping
    //color = color / (color + vec3(1.0));
    // gamma correct
    //color = pow(color, vec3(1.0/2.2)); 

    //shadowValue /= float(numShadowsDepth);
    //color += (1.0 - shadowValue) * 0.5;

    FragColor = vec4(color, 1.0);
    //FragColor.rgb = prefilteredColor;

    //FragColorBloom = vec4(0.0);

    //FragColor.rgb *= vec3(0.5, 0.8, 0.6);

    if(UseBloom) {
       // FragColorBloom = Tex2D(Bloom, TexCoords, UseBloomTriplanar);
    }

    if(UseAlpha) {
        if(albedoFullColor.a < AphaThreshold) {
            discard;
        }
    }
}