static const float shadowDepthTextureSize = 1024.0f;

cbuffer cbuffer_scene : register(b0, space0) {
  uint4 scene[9];
};
Texture2D shadowMap : register(t1, space0);
SamplerComparisonState shadowSampler : register(s2, space0);

struct FragmentInput {
  float3 shadowPos;
  float3 fragPos;
  float3 fragNorm;
};

static const float3 albedo = float3(0.899999976f, 0.899999976f, 0.899999976f);
static const float ambientFactor = 0.200000003f;

struct tint_symbol_1 {
  float3 shadowPos : TEXCOORD0;
  float3 fragPos : TEXCOORD1;
  float3 fragNorm : TEXCOORD2;
};
struct tint_symbol_2 {
  float4 value : SV_Target0;
};

float4 main_inner(FragmentInput input) {
  float visibility = 0.0f;
  const float oneOverShadowDepthTextureSize = (1.0f / shadowDepthTextureSize);
  {
    [loop] for(int y = -1; (y <= 1); y = (y + 1)) {
      {
        [loop] for(int x = -1; (x <= 1); x = (x + 1)) {
          const float2 offset = float2((float(x) * oneOverShadowDepthTextureSize), (float(y) * oneOverShadowDepthTextureSize));
          visibility = (visibility + shadowMap.SampleCmp(shadowSampler, (input.shadowPos.xy + offset), (input.shadowPos.z - 0.007f)));
        }
      }
    }
  }
  visibility = (visibility / 9.0f);
  const float lambertFactor = max(dot(normalize((asfloat(scene[8].xyz) - input.fragPos)), input.fragNorm), 0.0f);
  const float lightingFactor = min((ambientFactor + (visibility * lambertFactor)), 1.0f);
  return float4((lightingFactor * albedo), 1.0f);
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const FragmentInput tint_symbol_4 = {tint_symbol.shadowPos, tint_symbol.fragPos, tint_symbol.fragNorm};
  const float4 inner_result = main_inner(tint_symbol_4);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
