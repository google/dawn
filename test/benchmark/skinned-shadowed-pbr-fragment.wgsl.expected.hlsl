benchmark/skinned-shadowed-pbr-fragment.wgsl:51:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
  lights : @stride(32) array<Light>;
            ^^^^^^

static const float GAMMA = 2.200000048f;

float3 linearTosRGB(float3 tint_symbol) {
  const float INV_GAMMA = (1.0f / GAMMA);
  return pow(tint_symbol, float3((INV_GAMMA).xxx));
}

float3 sRGBToLinear(float3 srgb) {
  return pow(srgb, float3((GAMMA).xxx));
}

cbuffer cbuffer_camera : register(b0, space0) {
  uint4 camera[14];
};

ByteAddressBuffer clusterLights : register(t1, space0);

ByteAddressBuffer globalLights : register(t2, space0);
static const uint3 tileCount = uint3(32u, 18u, 48u);

float linearDepth(float depthSample) {
  return ((asfloat(camera[13].w) * asfloat(camera[13].z)) / mad(depthSample, (asfloat(camera[13].z) - asfloat(camera[13].w)), asfloat(camera[13].w)));
}

uint3 getTile(float4 fragCoord) {
  const float sliceScale = (float(tileCount.z) / log2((asfloat(camera[13].w) / asfloat(camera[13].z))));
  const float sliceBias = -(((float(tileCount.z) * log2(asfloat(camera[13].z))) / log2((asfloat(camera[13].w) / asfloat(camera[13].z)))));
  const uint zTile = uint(max(((log2(linearDepth(fragCoord.z)) * sliceScale) + sliceBias), 0.0f));
  return uint3(uint((fragCoord.x / (asfloat(camera[13].x) / float(tileCount.x)))), uint((fragCoord.y / (asfloat(camera[13].y) / float(tileCount.y)))), zTile);
}

uint getClusterIndex(float4 fragCoord) {
  const uint3 tile = getTile(fragCoord);
  return ((tile.x + (tile.y * tileCount.x)) + ((tile.z * tileCount.x) * tileCount.y));
}

SamplerState defaultSampler : register(s3, space0);
Texture2D shadowTexture : register(t4, space0);
SamplerComparisonState shadowSampler : register(s5, space0);

ByteAddressBuffer lightShadowTable : register(t6, space0);
static float2 shadowSampleOffsets[16] = {float2(-1.5f, -1.5f), float2(-1.5f, -0.5f), float2(-1.5f, 0.5f), float2(-1.5f, 1.5f), float2(-0.5f, -1.5f), float2(-0.5f, -0.5f), float2(-0.5f, 0.5f), float2(-0.5f, 1.5f), float2(0.5f, -1.5f), float2(0.5f, -0.5f), float2(0.5f, 0.5f), float2(0.5f, 1.5f), float2(1.5f, -1.5f), float2(1.5f, -0.5f), float2(1.5f, 0.5f), float2(1.5f, 1.5f)};
static const uint shadowSampleCount = 16u;

ByteAddressBuffer shadow : register(t7, space0);

float4x4 tint_symbol_8(ByteAddressBuffer buffer, uint offset) {
  return float4x4(asfloat(buffer.Load4((offset + 0u))), asfloat(buffer.Load4((offset + 16u))), asfloat(buffer.Load4((offset + 32u))), asfloat(buffer.Load4((offset + 48u))));
}

float dirLightVisibility(float3 worldPos) {
  const int shadowIndex = asint(lightShadowTable.Load(0u));
  if ((shadowIndex == -1)) {
    return 1.0f;
  }
  const float4 viewport = asfloat(shadow.Load4((80u * uint(shadowIndex))));
  const float4 lightPos = mul(float4(worldPos, 1.0f), tint_symbol_8(shadow, ((80u * uint(shadowIndex)) + 16u)));
  const float3 shadowPos = float3((((lightPos.xy / lightPos.w) * float2(0.5f, -0.5f)) + float2(0.5f, 0.5f)), (lightPos.z / lightPos.w));
  const float2 viewportPos = float2((viewport.xy + (shadowPos.xy * viewport.zw)));
  int3 tint_tmp;
  shadowTexture.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  const float2 texelSize = (1.0f / float2(tint_tmp.xy));
  const float4 clampRect = float4((viewport.xy - texelSize), ((viewport.xy + viewport.zw) + texelSize));
  float visibility = 0.0f;
  {
    [loop] for(uint i = 0u; (i < shadowSampleCount); i = (i + 1u)) {
      visibility = (visibility + shadowTexture.SampleCmpLevelZero(shadowSampler, clamp((viewportPos + (shadowSampleOffsets[i] * texelSize)), clampRect.xy, clampRect.zw), (shadowPos.z - 0.003f)));
    }
  }
  return (visibility / float(shadowSampleCount));
}

int getCubeFace(float3 v) {
  const float3 vAbs = abs(v);
  bool tint_tmp_1 = (vAbs.z >= vAbs.x);
  if (tint_tmp_1) {
    tint_tmp_1 = (vAbs.z >= vAbs.y);
  }
  if ((tint_tmp_1)) {
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

float pointLightVisibility(uint lightIndex, float3 worldPos, float3 pointToLight) {
  int shadowIndex = asint(lightShadowTable.Load((4u * (lightIndex + 1u))));
  if ((shadowIndex == -1)) {
    return 1.0f;
  }
  shadowIndex = (shadowIndex + getCubeFace((pointToLight * -1.0f)));
  const float4 viewport = asfloat(shadow.Load4((80u * uint(shadowIndex))));
  const float4 lightPos = mul(float4(worldPos, 1.0f), tint_symbol_8(shadow, ((80u * uint(shadowIndex)) + 16u)));
  const float3 shadowPos = float3((((lightPos.xy / lightPos.w) * float2(0.5f, -0.5f)) + float2(0.5f, 0.5f)), (lightPos.z / lightPos.w));
  const float2 viewportPos = float2((viewport.xy + (shadowPos.xy * viewport.zw)));
  int3 tint_tmp_2;
  shadowTexture.GetDimensions(0, tint_tmp_2.x, tint_tmp_2.y, tint_tmp_2.z);
  const float2 texelSize = (1.0f / float2(tint_tmp_2.xy));
  const float4 clampRect = float4(viewport.xy, (viewport.xy + viewport.zw));
  float visibility = 0.0f;
  {
    [loop] for(uint i = 0u; (i < shadowSampleCount); i = (i + 1u)) {
      visibility = (visibility + shadowTexture.SampleCmpLevelZero(shadowSampler, clamp((viewportPos + (shadowSampleOffsets[i] * texelSize)), clampRect.xy, clampRect.zw), (shadowPos.z - 0.01f)));
    }
  }
  return (visibility / float(shadowSampleCount));
}

struct VertexOutput {
  float4 position;
  float3 worldPos;
  float3 view;
  float2 texcoord;
  float2 texcoord2;
  float4 color;
  float4 instanceColor;
  float3 normal;
  float3 tangent;
  float3 bitangent;
};

cbuffer cbuffer_material : register(b8, space0) {
  uint4 material[3];
};
Texture2D<float4> baseColorTexture : register(t9, space0);
SamplerState baseColorSampler : register(s10, space0);
Texture2D<float4> normalTexture : register(t11, space0);
SamplerState normalSampler : register(s12, space0);
Texture2D<float4> metallicRoughnessTexture : register(t13, space0);
SamplerState metallicRoughnessSampler : register(s14, space0);
Texture2D<float4> occlusionTexture : register(t15, space0);
SamplerState occlusionSampler : register(s16, space0);
Texture2D<float4> emissiveTexture : register(t17, space0);
SamplerState emissiveSampler : register(s18, space0);

struct SurfaceInfo {
  float4 baseColor;
  float3 albedo;
  float metallic;
  float roughness;
  float3 normal;
  float3 f0;
  float ao;
  float3 emissive;
  float3 v;
};

SurfaceInfo GetSurfaceInfo(VertexOutput input) {
  if (true) {
    SurfaceInfo surface = (SurfaceInfo)0;
    surface.v = normalize(input.view);
    const float3x3 tbn = float3x3(input.tangent, input.bitangent, input.normal);
    const float3 normalMap = normalTexture.Sample(normalSampler, input.texcoord).rgb;
    surface.normal = normalize(mul(((2.0f * normalMap) - float3((1.0f).xxx)), tbn));
    const float4 baseColorMap = baseColorTexture.Sample(baseColorSampler, input.texcoord);
    surface.baseColor = ((input.color * asfloat(material[0])) * baseColorMap);
    if ((surface.baseColor.a < asfloat(material[2].z))) {
      discard;
    }
    surface.albedo = surface.baseColor.rgb;
    const float4 metallicRoughnessMap = metallicRoughnessTexture.Sample(metallicRoughnessSampler, input.texcoord);
    surface.metallic = (asfloat(material[2].x) * metallicRoughnessMap.b);
    surface.roughness = (asfloat(material[2].y) * metallicRoughnessMap.g);
    const float3 dielectricSpec = float3((0.039999999f).xxx);
    surface.f0 = lerp(dielectricSpec, surface.albedo, float3((surface.metallic).xxx));
    const float4 occlusionMap = occlusionTexture.Sample(occlusionSampler, input.texcoord);
    surface.ao = (asfloat(material[1].w) * occlusionMap.r);
    const float4 emissiveMap = emissiveTexture.Sample(emissiveSampler, input.texcoord);
    surface.emissive = (asfloat(material[1].xyz) * emissiveMap.rgb);
    if ((input.instanceColor.a == 0.0f)) {
      surface.albedo = (surface.albedo + input.instanceColor.rgb);
    } else {
      surface.albedo = (surface.albedo * input.instanceColor.rgb);
    }
    return surface;
  }
  SurfaceInfo unused;
  return unused;
}

static const float PI = 3.141592741f;
static const uint LightType_Point = 0u;
static const uint LightType_Spot = 1u;
static const uint LightType_Directional = 2u;

struct PuctualLight {
  uint lightType;
  float3 pointToLight;
  float range;
  float3 color;
  float intensity;
};

float3 FresnelSchlick(float cosTheta, float3 F0) {
  return (F0 + ((float3((1.0f).xxx) - F0) * pow((1.0f - cosTheta), 5.0f)));
}

float DistributionGGX(float3 N, float3 H, float roughness) {
  const float a_1 = (roughness * roughness);
  const float a2 = (a_1 * a_1);
  const float NdotH = max(dot(N, H), 0.0f);
  const float NdotH2 = (NdotH * NdotH);
  const float num = a2;
  const float denom = ((NdotH2 * (a2 - 1.0f)) + 1.0f);
  return (num / ((PI * denom) * denom));
}

float GeometrySchlickGGX(float NdotV, float roughness) {
  const float r_1 = (roughness + 1.0f);
  const float k = ((r_1 * r_1) / 8.0f);
  const float num = NdotV;
  const float denom = ((NdotV * (1.0f - k)) + k);
  return (num / denom);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
  const float NdotV = max(dot(N, V), 0.0f);
  const float NdotL = max(dot(N, L), 0.0f);
  const float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  const float ggx1 = GeometrySchlickGGX(NdotL, roughness);
  return (ggx1 * ggx2);
}

float lightAttenuation(PuctualLight light) {
  if ((light.lightType == LightType_Directional)) {
    return 1.0f;
  }
  const float distance = length(light.pointToLight);
  if ((light.range <= 0.0f)) {
    return (1.0f / pow(distance, 2.0f));
  }
  return (clamp((1.0f - pow((distance / light.range), 4.0f)), 0.0f, 1.0f) / pow(distance, 2.0f));
}

float3 lightRadiance(PuctualLight light, SurfaceInfo surface) {
  const float3 L = normalize(light.pointToLight);
  const float3 H = normalize((surface.v + L));
  const float NDF = DistributionGGX(surface.normal, H, surface.roughness);
  const float G = GeometrySmith(surface.normal, surface.v, L, surface.roughness);
  const float3 F = FresnelSchlick(max(dot(H, surface.v), 0.0f), surface.f0);
  const float3 kD = ((float3((1.0f).xxx) - F) * (1.0f - surface.metallic));
  const float NdotL = max(dot(surface.normal, L), 0.0f);
  const float3 numerator = ((NDF * G) * F);
  const float denominator = max(((4.0f * max(dot(surface.normal, surface.v), 0.0f)) * NdotL), 0.001f);
  const float3 specular = (numerator / float3((denominator).xxx));
  const float3 radiance = ((light.color * light.intensity) * lightAttenuation(light));
  return (((((kD * surface.albedo) / float3((PI).xxx)) + specular) * radiance) * NdotL);
}

Texture2D<float4> ssaoTexture : register(t19, space0);

struct FragmentOutput {
  float4 color;
  float4 emissive;
};
struct tint_symbol_3 {
  float3 worldPos : TEXCOORD0;
  float3 view : TEXCOORD1;
  float2 texcoord : TEXCOORD2;
  float2 texcoord2 : TEXCOORD3;
  float4 color : TEXCOORD4;
  float4 instanceColor : TEXCOORD5;
  float3 normal : TEXCOORD6;
  float3 tangent : TEXCOORD7;
  float3 bitangent : TEXCOORD8;
  float4 position : SV_Position;
};
struct tint_symbol_4 {
  float4 color : SV_Target0;
  float4 emissive : SV_Target1;
};

FragmentOutput fragmentMain_inner(VertexOutput input) {
  const SurfaceInfo surface = GetSurfaceInfo(input);
  float3 Lo = float3(0.0f, 0.0f, 0.0f);
  if ((asfloat(globalLights.Load(28u)) > 0.0f)) {
    PuctualLight light = (PuctualLight)0;
    light.lightType = LightType_Directional;
    light.pointToLight = asfloat(globalLights.Load3(32u));
    light.color = asfloat(globalLights.Load3(16u));
    light.intensity = asfloat(globalLights.Load(28u));
    const float lightVis = dirLightVisibility(input.worldPos);
    Lo = (Lo + (lightRadiance(light, surface) * lightVis));
  }
  const uint clusterIndex = getClusterIndex(input.position);
  const uint lightOffset = clusterLights.Load((4u + (8u * clusterIndex)));
  const uint lightCount = clusterLights.Load(((4u + (8u * clusterIndex)) + 4u));
  {
    [loop] for(uint lightIndex = 0u; (lightIndex < lightCount); lightIndex = (lightIndex + 1u)) {
      const uint i = clusterLights.Load((221188u + (4u * (lightOffset + lightIndex))));
      PuctualLight light = (PuctualLight)0;
      light.lightType = LightType_Point;
      light.pointToLight = (asfloat(globalLights.Load3((48u + (32u * i)))).xyz - input.worldPos);
      light.range = asfloat(globalLights.Load(((48u + (32u * i)) + 12u)));
      light.color = asfloat(globalLights.Load3(((48u + (32u * i)) + 16u)));
      light.intensity = asfloat(globalLights.Load(((48u + (32u * i)) + 28u)));
      const float lightVis = pointLightVisibility(i, input.worldPos, light.pointToLight);
      Lo = (Lo + (lightRadiance(light, surface) * lightVis));
    }
  }
  int2 tint_tmp_3;
  ssaoTexture.GetDimensions(tint_tmp_3.x, tint_tmp_3.y);
  const float2 ssaoCoord = (input.position.xy / float2(tint_tmp_3.xy));
  const float ssaoFactor = ssaoTexture.Sample(defaultSampler, ssaoCoord).r;
  const float3 ambient = (((asfloat(globalLights.Load3(0u)) * surface.albedo) * surface.ao) * ssaoFactor);
  const float3 color = linearTosRGB(((Lo + ambient) + surface.emissive));
  FragmentOutput tint_symbol_1 = (FragmentOutput)0;
  tint_symbol_1.color = float4(color, surface.baseColor.a);
  tint_symbol_1.emissive = float4(surface.emissive, surface.baseColor.a);
  return tint_symbol_1;
}

tint_symbol_4 fragmentMain(tint_symbol_3 tint_symbol_2) {
  const VertexOutput tint_symbol_15 = {tint_symbol_2.position, tint_symbol_2.worldPos, tint_symbol_2.view, tint_symbol_2.texcoord, tint_symbol_2.texcoord2, tint_symbol_2.color, tint_symbol_2.instanceColor, tint_symbol_2.normal, tint_symbol_2.tangent, tint_symbol_2.bitangent};
  const FragmentOutput inner_result = fragmentMain_inner(tint_symbol_15);
  tint_symbol_4 wrapper_result = (tint_symbol_4)0;
  wrapper_result.color = inner_result.color;
  wrapper_result.emissive = inner_result.emissive;
  return wrapper_result;
}
