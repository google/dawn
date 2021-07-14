struct FragmentInput {
  float2 vUv;
};
struct FragmentOutput {
  float4 color;
};

Texture2D depthMap : register(t5, space1);
SamplerState texSampler : register(s3, space1);

struct tint_symbol_2 {
  float2 vUv : TEXCOORD2;
};
struct tint_symbol_3 {
  float4 color : SV_Target0;
};

tint_symbol_3 main(tint_symbol_2 tint_symbol_1) {
  const FragmentInput fIn = {tint_symbol_1.vUv};
  const float tint_symbol = depthMap.Sample(texSampler, fIn.vUv).x;
  const float3 color = float3(tint_symbol, tint_symbol, tint_symbol);
  FragmentOutput fOut = (FragmentOutput)0;
  fOut.color = float4(color, 1.0f);
  const tint_symbol_3 tint_symbol_4 = {fOut.color};
  return tint_symbol_4;
}
