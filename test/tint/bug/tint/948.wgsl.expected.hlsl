cbuffer cbuffer_x_20 : register(b9, space2) {
  uint4 x_20[8];
};
Texture2D<float4> frameMapTexture : register(t3, space2);
SamplerState frameMapSampler : register(s2, space2);
static float2 tUV = float2(0.0f, 0.0f);
Texture2D<float4> tileMapsTexture0 : register(t5, space2);
SamplerState tileMapsSampler : register(s4, space2);
Texture2D<float4> tileMapsTexture1 : register(t6, space2);
Texture2D<float4> animationMapTexture : register(t8, space2);
SamplerState animationMapSampler : register(s7, space2);
static float mt = 0.0f;
Texture2D<float4> spriteSheetTexture : register(t1, space2);
SamplerState spriteSheetSampler : register(s0, space2);
static float4 glFragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float2 tileID_1 = float2(0.0f, 0.0f);
static float2 levelUnits = float2(0.0f, 0.0f);
static float2 stageUnits_1 = float2(0.0f, 0.0f);
static float3 vPosition = float3(0.0f, 0.0f, 0.0f);
static float2 vUV = float2(0.0f, 0.0f);

float4x4 getFrameData_f1_(inout float frameID) {
  float fX = 0.0f;
  const float x_15 = frameID;
  const float x_25 = asfloat(x_20[6].w);
  fX = (x_15 / x_25);
  const float4 x_40 = frameMapTexture.SampleBias(frameMapSampler, float2(fX, 0.0f), 0.0f);
  const float4 x_47 = frameMapTexture.SampleBias(frameMapSampler, float2(fX, 0.25f), 0.0f);
  const float4 x_54 = frameMapTexture.SampleBias(frameMapSampler, float2(fX, 0.5f), 0.0f);
  return float4x4(float4(x_40.x, x_40.y, x_40.z, x_40.w), float4(x_47.x, x_47.y, x_47.z, x_47.w), float4(x_54.x, x_54.y, x_54.z, x_54.w), float4(float4(0.0f, 0.0f, 0.0f, 0.0f).x, float4(0.0f, 0.0f, 0.0f, 0.0f).y, float4(0.0f, 0.0f, 0.0f, 0.0f).z, float4(0.0f, 0.0f, 0.0f, 0.0f).w));
}

void main_1() {
  float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2 tileUV = float2(0.0f, 0.0f);
  float2 tileID = float2(0.0f, 0.0f);
  float2 sheetUnits = float2(0.0f, 0.0f);
  float spriteUnits = 0.0f;
  float2 stageUnits = float2(0.0f, 0.0f);
  int i = 0;
  float frameID_1 = 0.0f;
  float4 animationData = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float f = 0.0f;
  float4x4 frameData = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float param = 0.0f;
  float2 frameSize = float2(0.0f, 0.0f);
  float2 offset_1 = float2(0.0f, 0.0f);
  float2 ratio = float2(0.0f, 0.0f);
  float4 nc = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float alpha = 0.0f;
  float3 mixed = float3(0.0f, 0.0f, 0.0f);
  color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  tileUV = frac(tUV);
  const float x_91 = tileUV.y;
  tileUV.y = (1.0f - x_91);
  tileID = floor(tUV);
  const float2 x_101 = asfloat(x_20[6].xy);
  sheetUnits = (float2(1.0f, 1.0f) / x_101);
  const float x_106 = asfloat(x_20[6].w);
  spriteUnits = (1.0f / x_106);
  const float2 x_111 = asfloat(x_20[5].zw);
  stageUnits = (float2(1.0f, 1.0f) / x_111);
  i = 0;
  {
    [loop] for(; (i < 2); i = (i + 1)) {
      switch(i) {
        case 1: {
          const float2 x_150 = tileID;
          const float2 x_154 = asfloat(x_20[5].zw);
          const float4 x_156 = tileMapsTexture1.SampleBias(tileMapsSampler, ((x_150 + float2(0.5f, 0.5f)) / x_154), 0.0f);
          frameID_1 = x_156.x;
          break;
        }
        case 0: {
          const float2 x_136 = tileID;
          const float2 x_140 = asfloat(x_20[5].zw);
          const float4 x_142 = tileMapsTexture0.SampleBias(tileMapsSampler, ((x_136 + float2(0.5f, 0.5f)) / x_140), 0.0f);
          frameID_1 = x_142.x;
          break;
        }
        default: {
          break;
        }
      }
      const float x_166 = frameID_1;
      const float x_169 = asfloat(x_20[6].w);
      const float4 x_172 = animationMapTexture.SampleBias(animationMapSampler, float2(((x_166 + 0.5f) / x_169), 0.0f), 0.0f);
      animationData = x_172;
      const float x_174 = animationData.y;
      if ((x_174 > 0.0f)) {
        const float x_181 = asfloat(x_20[0].x);
        const float x_184 = animationData.z;
        mt = ((x_181 * x_184) % 1.0f);
        f = 0.0f;
        {
          [loop] for(; (f < 8.0f); f = (f + 1.0f)) {
            const float x_197 = animationData.y;
            if ((x_197 > mt)) {
              const float x_203 = animationData.x;
              frameID_1 = x_203;
              break;
            }
            const float x_208 = frameID_1;
            const float x_211 = asfloat(x_20[6].w);
            const float4 x_217 = animationMapTexture.SampleBias(animationMapSampler, float2(((x_208 + 0.5f) / x_211), (0.125f * f)), 0.0f);
            animationData = x_217;
          }
        }
      }
      param = (frameID_1 + 0.5f);
      const float4x4 x_225 = getFrameData_f1_(param);
      frameData = x_225;
      const float4 x_228 = frameData[0];
      const float2 x_231 = asfloat(x_20[6].xy);
      frameSize = (float2(x_228.w, x_228.z) / x_231);
      const float4 x_235 = frameData[0];
      offset_1 = (float2(x_235.x, x_235.y) * sheetUnits);
      const float4 x_241 = frameData[2];
      const float4 x_244 = frameData[0];
      ratio = (float2(x_241.x, x_241.y) / float2(x_244.w, x_244.z));
      const float x_248 = frameData[2].z;
      if ((x_248 == 1.0f)) {
        const float2 x_252 = tileUV;
        tileUV = float2(x_252.y, x_252.x);
      }
      if ((i == 0)) {
        const float4 x_268 = spriteSheetTexture.Sample(spriteSheetSampler, ((tileUV * frameSize) + offset_1));
        color = x_268;
      } else {
        const float4 x_279 = spriteSheetTexture.Sample(spriteSheetSampler, ((tileUV * frameSize) + offset_1));
        nc = x_279;
        const float x_283 = color.w;
        const float x_285 = nc.w;
        alpha = min((x_283 + x_285), 1.0f);
        const float4 x_290 = color;
        const float4 x_292 = nc;
        const float x_295 = nc.w;
        mixed = lerp(float3(x_290.x, x_290.y, x_290.z), float3(x_292.x, x_292.y, x_292.z), float3(x_295, x_295, x_295));
        const float3 x_298 = mixed;
        color = float4(x_298.x, x_298.y, x_298.z, alpha);
      }
    }
  }
  const float3 x_310 = asfloat(x_20[7].xyz);
  const float4 x_311 = color;
  const float3 x_313 = (float3(x_311.x, x_311.y, x_311.z) * x_310);
  color = float4(x_313.x, x_313.y, x_313.z, color.w);
  glFragColor = color;
  return;
}

struct main_out {
  float4 glFragColor_1;
};
struct tint_symbol_1 {
  float3 vPosition_param : TEXCOORD0;
  float2 vUV_param : TEXCOORD1;
  float2 tUV_param : TEXCOORD2;
  float2 stageUnits_1_param : TEXCOORD3;
  float2 levelUnits_param : TEXCOORD4;
  float2 tileID_1_param : TEXCOORD5;
};
struct tint_symbol_2 {
  float4 glFragColor_1 : SV_Target0;
};

main_out main_inner(float2 tUV_param, float2 tileID_1_param, float2 levelUnits_param, float2 stageUnits_1_param, float3 vPosition_param, float2 vUV_param) {
  tUV = tUV_param;
  tileID_1 = tileID_1_param;
  levelUnits = levelUnits_param;
  stageUnits_1 = stageUnits_1_param;
  vPosition = vPosition_param;
  vUV = vUV_param;
  main_1();
  const main_out tint_symbol_6 = {glFragColor};
  return tint_symbol_6;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.tUV_param, tint_symbol.tileID_1_param, tint_symbol.levelUnits_param, tint_symbol.stageUnits_1_param, tint_symbol.vPosition_param, tint_symbol.vUV_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.glFragColor_1 = inner_result.glFragColor_1;
  return wrapper_result;
}
