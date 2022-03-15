benchmark/skinned-shadowed-pbr-fragment.wgsl:51:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
  lights : @stride(32) array<Light>;
            ^^^^^^

#version 310 es
precision mediump float;

layout(location = 0) in vec3 worldPos_1;
layout(location = 1) in vec3 view_1;
layout(location = 2) in vec2 texcoord_1;
layout(location = 3) in vec2 texcoord2_1;
layout(location = 4) in vec4 color_1;
layout(location = 5) in vec4 instanceColor_1;
layout(location = 6) in vec3 normal_1;
layout(location = 7) in vec3 tangent_1;
layout(location = 8) in vec3 bitangent_1;
layout(location = 0) out vec4 color_2;
layout(location = 1) out vec4 emissive_1;
const float GAMMA = 2.200000048f;
vec3 linearTosRGB(vec3 linear) {
  float INV_GAMMA = (1.0f / GAMMA);
  return pow(linear, vec3(INV_GAMMA));
}

struct Camera {
  mat4 projection;
  mat4 inverseProjection;
  mat4 view;
  vec3 position;
  float time;
  vec2 outputSize;
  float zNear;
  float zFar;
};

layout(binding = 0) uniform Camera_1 {
  mat4 projection;
  mat4 inverseProjection;
  mat4 view;
  vec3 position;
  float time;
  vec2 outputSize;
  float zNear;
  float zFar;
} camera;

struct ClusterLights {
  uint offset;
  uint count;
};

struct ClusterLightGroup {
  uint offset;
  ClusterLights lights[27648];
  uint indices[1769472];
};

layout(binding = 1, std430) buffer ClusterLightGroup_1 {
  uint offset;
  ClusterLights lights[27648];
  uint indices[1769472];
} clusterLights;
struct Light {
  vec3 position;
  float range;
  vec3 color;
  float intensity;
};

layout(binding = 2, std430) buffer GlobalLights_1 {
  vec3 ambient;
  vec3 dirColor;
  float dirIntensity;
  vec3 dirDirection;
  uint lightCount;
  Light lights[];
} globalLights;
const uvec3 tileCount = uvec3(32u, 18u, 48u);
float linearDepth(float depthSample) {
  return ((camera.zFar * camera.zNear) / ((depthSample) * ((camera.zNear - camera.zFar)) + (camera.zFar)));
}

uvec3 getTile(vec4 fragCoord) {
  float sliceScale = (float(tileCount.z) / log2((camera.zFar / camera.zNear)));
  float sliceBias = -(((float(tileCount.z) * log2(camera.zNear)) / log2((camera.zFar / camera.zNear))));
  float tint_symbol_3 = linearDepth(fragCoord.z);
  float tint_symbol_4 = log2(tint_symbol_3);
  float tint_symbol_5 = max(((tint_symbol_4 * sliceScale) + sliceBias), 0.0f);
  uint zTile = uint(tint_symbol_5);
  return uvec3(uint((fragCoord.x / (camera.outputSize.x / float(tileCount.x)))), uint((fragCoord.y / (camera.outputSize.y / float(tileCount.y)))), zTile);
}

uint getClusterIndex(vec4 fragCoord) {
  uvec3 tile = getTile(fragCoord);
  return ((tile.x + (tile.y * tileCount.x)) + ((tile.z * tileCount.x) * tileCount.y));
}

layout(binding = 6, std430) buffer LightShadowTable_1 {
  int light[];
} lightShadowTable;
vec2 shadowSampleOffsets[16] = vec2[16](vec2(-1.5f, -1.5f), vec2(-1.5f, -0.5f), vec2(-1.5f, 0.5f), vec2(-1.5f, 1.5f), vec2(-0.5f, -1.5f), vec2(-0.5f, -0.5f), vec2(-0.5f, 0.5f), vec2(-0.5f, 1.5f), vec2(0.5f, -1.5f), vec2(0.5f, -0.5f), vec2(0.5f, 0.5f), vec2(0.5f, 1.5f), vec2(1.5f, -1.5f), vec2(1.5f, -0.5f), vec2(1.5f, 0.5f), vec2(1.5f, 1.5f));
const uint shadowSampleCount = 16u;
struct ShadowProperties {
  vec4 viewport;
  mat4 viewProj;
};

layout(binding = 7, std430) buffer LightShadows_1 {
  ShadowProperties properties[];
} shadow;
uniform highp sampler2D shadowTexture_1;
uniform highp sampler2DShadow shadowTexture_shadowSampler;

float dirLightVisibility(vec3 worldPos) {
  int shadowIndex = lightShadowTable.light[0u];
  if ((shadowIndex == -1)) {
    return 1.0f;
  }
  vec4 viewport = shadow.properties[shadowIndex].viewport;
  vec4 lightPos = (shadow.properties[shadowIndex].viewProj * vec4(worldPos, 1.0f));
  vec3 shadowPos = vec3((((lightPos.xy / lightPos.w) * vec2(0.5f, -0.5f)) + vec2(0.5f, 0.5f)), (lightPos.z / lightPos.w));
  vec2 viewportPos = vec2((viewport.xy + (shadowPos.xy * viewport.zw)));
  vec2 texelSize = (1.0f / vec2(textureSize(shadowTexture_1, 0)));
  vec4 clampRect = vec4((viewport.xy - texelSize), ((viewport.xy + viewport.zw) + texelSize));
  float visibility = 0.0f;
  {
    for(uint i = 0u; (i < shadowSampleCount); i = (i + 1u)) {
      visibility = (visibility + texture(shadowTexture_shadowSampler, vec3(clamp((viewportPos + (shadowSampleOffsets[i] * texelSize)), clampRect.xy, clampRect.zw), (shadowPos.z - 0.003f))));
    }
  }
  return (visibility / float(shadowSampleCount));
}

int getCubeFace(vec3 v) {
  vec3 vAbs = abs(v);
  bool tint_tmp = (vAbs.z >= vAbs.x);
  if (tint_tmp) {
    tint_tmp = (vAbs.z >= vAbs.y);
  }
  if ((tint_tmp)) {
    if ((v.z < 0.0f)) {
      return 5;
    }
    return 4;
  }
  if ((vAbs.y >= vAbs.x)) {
    if ((v.y < 0.0f)) {
      return 3;
    }
    return 2;
  }
  if ((v.x < 0.0f)) {
    return 1;
  }
  return 0;
}

float pointLightVisibility(uint lightIndex, vec3 worldPos, vec3 pointToLight) {
  int shadowIndex = lightShadowTable.light[(lightIndex + 1u)];
  if ((shadowIndex == -1)) {
    return 1.0f;
  }
  int tint_symbol_6 = shadowIndex;
  int tint_symbol_7 = getCubeFace((pointToLight * -1.0f));
  shadowIndex = (tint_symbol_6 + tint_symbol_7);
  vec4 viewport = shadow.properties[shadowIndex].viewport;
  vec4 lightPos = (shadow.properties[shadowIndex].viewProj * vec4(worldPos, 1.0f));
  vec3 shadowPos = vec3((((lightPos.xy / lightPos.w) * vec2(0.5f, -0.5f)) + vec2(0.5f, 0.5f)), (lightPos.z / lightPos.w));
  vec2 viewportPos = vec2((viewport.xy + (shadowPos.xy * viewport.zw)));
  vec2 texelSize = (1.0f / vec2(textureSize(shadowTexture_1, 0)));
  vec4 clampRect = vec4(viewport.xy, (viewport.xy + viewport.zw));
  float visibility = 0.0f;
  {
    for(uint i = 0u; (i < shadowSampleCount); i = (i + 1u)) {
      visibility = (visibility + texture(shadowTexture_shadowSampler, vec3(clamp((viewportPos + (shadowSampleOffsets[i] * texelSize)), clampRect.xy, clampRect.zw), (shadowPos.z - 0.01f))));
    }
  }
  return (visibility / float(shadowSampleCount));
}

struct VertexOutput {
  vec4 position;
  vec3 worldPos;
  vec3 view;
  vec2 texcoord;
  vec2 texcoord2;
  vec4 color;
  vec4 instanceColor;
  vec3 normal;
  vec3 tangent;
  vec3 bitangent;
};

struct Material {
  vec4 baseColorFactor;
  vec3 emissiveFactor;
  float occlusionStrength;
  vec2 metallicRoughnessFactor;
  float alphaCutoff;
};

layout(binding = 8) uniform Material_1 {
  vec4 baseColorFactor;
  vec3 emissiveFactor;
  float occlusionStrength;
  vec2 metallicRoughnessFactor;
  float alphaCutoff;
} material;

struct SurfaceInfo {
  vec4 baseColor;
  vec3 albedo;
  float metallic;
  float roughness;
  vec3 normal;
  vec3 f0;
  float ao;
  vec3 emissive;
  vec3 v;
};

uniform highp sampler2D normalTexture_normalSampler;
uniform highp sampler2D baseColorTexture_baseColorSampler;
uniform highp sampler2D metallicRoughnessTexture_metallicRoughnessSampler;
uniform highp sampler2D occlusionTexture_occlusionSampler;
uniform highp sampler2D emissiveTexture_emissiveSampler;

SurfaceInfo GetSurfaceInfo(VertexOutput tint_symbol) {
  SurfaceInfo surface = SurfaceInfo(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 0.0f, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
  surface.v = normalize(tint_symbol.view);
  mat3 tbn = mat3(tint_symbol.tangent, tint_symbol.bitangent, tint_symbol.normal);
  vec3 normalMap = texture(normalTexture_normalSampler, tint_symbol.texcoord).rgb;
  surface.normal = normalize((tbn * ((2.0f * normalMap) - vec3(1.0f))));
  vec4 baseColorMap = texture(baseColorTexture_baseColorSampler, tint_symbol.texcoord);
  surface.baseColor = ((tint_symbol.color * material.baseColorFactor) * baseColorMap);
  if ((surface.baseColor.a < material.alphaCutoff)) {
    discard;
  }
  surface.albedo = surface.baseColor.rgb;
  vec4 metallicRoughnessMap = texture(metallicRoughnessTexture_metallicRoughnessSampler, tint_symbol.texcoord);
  surface.metallic = (material.metallicRoughnessFactor.x * metallicRoughnessMap.b);
  surface.roughness = (material.metallicRoughnessFactor.y * metallicRoughnessMap.g);
  vec3 dielectricSpec = vec3(0.039999999f);
  surface.f0 = mix(dielectricSpec, surface.albedo, vec3(surface.metallic));
  vec4 occlusionMap = texture(occlusionTexture_occlusionSampler, tint_symbol.texcoord);
  surface.ao = (material.occlusionStrength * occlusionMap.r);
  vec4 emissiveMap = texture(emissiveTexture_emissiveSampler, tint_symbol.texcoord);
  surface.emissive = (material.emissiveFactor * emissiveMap.rgb);
  if ((tint_symbol.instanceColor.a == 0.0f)) {
    surface.albedo = (surface.albedo + tint_symbol.instanceColor.rgb);
  } else {
    surface.albedo = (surface.albedo * tint_symbol.instanceColor.rgb);
  }
  return surface;
}

const float PI = 3.141592741f;
const uint LightType_Point = 0u;
const uint LightType_Directional = 2u;
struct PuctualLight {
  uint lightType;
  vec3 pointToLight;
  float range;
  vec3 color;
  float intensity;
};

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
  return (F0 + ((vec3(1.0f) - F0) * pow((1.0f - cosTheta), 5.0f)));
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a_1 = (roughness * roughness);
  float a2 = (a_1 * a_1);
  float NdotH = max(dot(N, H), 0.0f);
  float NdotH2 = (NdotH * NdotH);
  float num = a2;
  float denom = ((NdotH2 * (a2 - 1.0f)) + 1.0f);
  return (num / ((PI * denom) * denom));
}

float GeometrySchlickGGX(float NdotV, float roughness) {
  float r_1 = (roughness + 1.0f);
  float k = ((r_1 * r_1) / 8.0f);
  float num = NdotV;
  float denom = ((NdotV * (1.0f - k)) + k);
  return (num / denom);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0f);
  float NdotL = max(dot(N, L), 0.0f);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);
  return (ggx1 * ggx2);
}

float lightAttenuation(PuctualLight light) {
  if ((light.lightType == LightType_Directional)) {
    return 1.0f;
  }
  float tint_symbol_1 = length(light.pointToLight);
  if ((light.range <= 0.0f)) {
    return (1.0f / pow(tint_symbol_1, 2.0f));
  }
  return (clamp((1.0f - pow((tint_symbol_1 / light.range), 4.0f)), 0.0f, 1.0f) / pow(tint_symbol_1, 2.0f));
}

vec3 lightRadiance(PuctualLight light, SurfaceInfo surface) {
  vec3 L = normalize(light.pointToLight);
  vec3 H = normalize((surface.v + L));
  float NDF = DistributionGGX(surface.normal, H, surface.roughness);
  float G = GeometrySmith(surface.normal, surface.v, L, surface.roughness);
  vec3 F = FresnelSchlick(max(dot(H, surface.v), 0.0f), surface.f0);
  vec3 kD = ((vec3(1.0f) - F) * (1.0f - surface.metallic));
  float NdotL = max(dot(surface.normal, L), 0.0f);
  vec3 numerator = ((NDF * G) * F);
  float denominator = max(((4.0f * max(dot(surface.normal, surface.v), 0.0f)) * NdotL), 0.001f);
  vec3 specular = (numerator / vec3(denominator));
  vec3 tint_symbol_8 = (light.color * light.intensity);
  float tint_symbol_9 = lightAttenuation(light);
  vec3 radiance = (tint_symbol_8 * tint_symbol_9);
  return (((((kD * surface.albedo) / vec3(PI)) + specular) * radiance) * NdotL);
}

struct FragmentOutput {
  vec4 color;
  vec4 emissive;
};

uniform highp sampler2D ssaoTexture_1;
uniform highp sampler2D ssaoTexture_defaultSampler;
FragmentOutput fragmentMain(VertexOutput tint_symbol) {
  SurfaceInfo surface = GetSurfaceInfo(tint_symbol);
  vec3 Lo = vec3(0.0f, 0.0f, 0.0f);
  if ((globalLights.dirIntensity > 0.0f)) {
    PuctualLight light = PuctualLight(0u, vec3(0.0f, 0.0f, 0.0f), 0.0f, vec3(0.0f, 0.0f, 0.0f), 0.0f);
    light.lightType = LightType_Directional;
    light.pointToLight = globalLights.dirDirection;
    light.color = globalLights.dirColor;
    light.intensity = globalLights.dirIntensity;
    float lightVis = dirLightVisibility(tint_symbol.worldPos);
    vec3 tint_symbol_10 = Lo;
    vec3 tint_symbol_11 = lightRadiance(light, surface);
    Lo = (tint_symbol_10 + (tint_symbol_11 * lightVis));
  }
  uint clusterIndex = getClusterIndex(tint_symbol.position);
  uint lightOffset = clusterLights.lights[clusterIndex].offset;
  uint lightCount = clusterLights.lights[clusterIndex].count;
  {
    for(uint lightIndex = 0u; (lightIndex < lightCount); lightIndex = (lightIndex + 1u)) {
      uint i = clusterLights.indices[(lightOffset + lightIndex)];
      PuctualLight light = PuctualLight(0u, vec3(0.0f, 0.0f, 0.0f), 0.0f, vec3(0.0f, 0.0f, 0.0f), 0.0f);
      light.lightType = LightType_Point;
      light.pointToLight = (globalLights.lights[i].position.xyz - tint_symbol.worldPos);
      light.range = globalLights.lights[i].range;
      light.color = globalLights.lights[i].color;
      light.intensity = globalLights.lights[i].intensity;
      float lightVis = pointLightVisibility(i, tint_symbol.worldPos, light.pointToLight);
      vec3 tint_symbol_12 = Lo;
      vec3 tint_symbol_13 = lightRadiance(light, surface);
      Lo = (tint_symbol_12 + (tint_symbol_13 * lightVis));
    }
  }
  vec2 ssaoCoord = (tint_symbol.position.xy / vec2(textureSize(ssaoTexture_1, 0).xy));
  float ssaoFactor = texture(ssaoTexture_defaultSampler, ssaoCoord).r;
  vec3 ambient = (((globalLights.ambient * surface.albedo) * surface.ao) * ssaoFactor);
  vec3 color = linearTosRGB(((Lo + ambient) + surface.emissive));
  FragmentOutput tint_symbol_2 = FragmentOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol_2.color = vec4(color, surface.baseColor.a);
  tint_symbol_2.emissive = vec4(surface.emissive, surface.baseColor.a);
  return tint_symbol_2;
}

void main() {
  VertexOutput tint_symbol_14 = VertexOutput(gl_FragCoord, worldPos_1, view_1, texcoord_1, texcoord2_1, color_1, instanceColor_1, normal_1, tangent_1, bitangent_1);
  FragmentOutput inner_result = fragmentMain(tint_symbol_14);
  color_2 = inner_result.color;
  emissive_1 = inner_result.emissive;
  return;
}
