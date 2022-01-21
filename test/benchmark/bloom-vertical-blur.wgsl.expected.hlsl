static const float2 bloomDir = float2(0.0f, 1.0f);
static float offsets[3] = {0.0f, 1.384615421f, 3.230769157f};
static float weights[3] = {0.227027029f, 0.31621623f, 0.07027027f};

cbuffer cbuffer_bloom : register(b0, space0) {
  uint4 bloom[1];
};
Texture2D<float4> bloomTexture : register(t1, space0);
SamplerState bloomSampler : register(s2, space0);

struct FragmentInput {
  float2 texCoord;
};

float4 getGaussianBlur(float2 texCoord) {
  int2 tint_tmp;
  bloomTexture.GetDimensions(tint_tmp.x, tint_tmp.y);
  const float2 texelRadius = (float2((asfloat(bloom[0].x)).xx) / float2(tint_tmp));
  const float2 step = (bloomDir * texelRadius);
  float4 sum = float4((0.0f).xxxx);
  sum = (sum + (bloomTexture.Sample(bloomSampler, texCoord) * weights[0]));
  sum = (sum + (bloomTexture.Sample(bloomSampler, (texCoord + (step * 1.0f))) * weights[1]));
  sum = (sum + (bloomTexture.Sample(bloomSampler, (texCoord - (step * 1.0f))) * weights[1]));
  sum = (sum + (bloomTexture.Sample(bloomSampler, (texCoord + (step * 2.0f))) * weights[2]));
  sum = (sum + (bloomTexture.Sample(bloomSampler, (texCoord - (step * 2.0f))) * weights[2]));
  return float4(sum.rgb, 1.0f);
}

Texture2D<float4> prevTexture : register(t3, space0);

struct tint_symbol_1 {
  float2 texCoord : TEXCOORD0;
};
struct tint_symbol_2 {
  float4 value : SV_Target0;
};

float4 fragmentMain_inner(FragmentInput input) {
  const float4 blurColor = getGaussianBlur(input.texCoord);
  const float4 dimColor = (prevTexture.Sample(bloomSampler, input.texCoord) * asfloat(bloom[0].y));
  return (blurColor + dimColor);
}

tint_symbol_2 fragmentMain(tint_symbol_1 tint_symbol) {
  const FragmentInput tint_symbol_4 = {tint_symbol.texCoord};
  const float4 inner_result = fragmentMain_inner(tint_symbol_4);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
