cbuffer cbuffer_uniforms : register(b0, space0) {
  uint4 uniforms[1];
};

struct VertexOutputs {
  float2 texcoords;
  float4 position;
};
struct tint_symbol_1 {
  uint VertexIndex : SV_VertexID;
};
struct tint_symbol_2 {
  float2 texcoords : TEXCOORD0;
  float4 position : SV_Position;
};

tint_symbol_2 vs_main(tint_symbol_1 tint_symbol) {
  const uint VertexIndex = tint_symbol.VertexIndex;
  float2 texcoord[3] = {float2(-0.5f, 0.0f), float2(1.5f, 0.0f), float2(0.5f, 2.0f)};
  VertexOutputs output = (VertexOutputs)0;
  output.position = float4(((texcoord[VertexIndex] * 2.0f) - float2(1.0f, 1.0f)), 0.0f, 1.0f);
  bool flipY = (asfloat(uniforms[0].y) < 0.0f);
  if (flipY) {
    output.texcoords = ((((texcoord[VertexIndex] * asfloat(uniforms[0].xy)) + asfloat(uniforms[0].zw)) * float2(1.0f, -1.0f)) + float2(0.0f, 1.0f));
  } else {
    output.texcoords = ((((texcoord[VertexIndex] * float2(1.0f, -1.0f)) + float2(0.0f, 1.0f)) * asfloat(uniforms[0].xy)) + asfloat(uniforms[0].zw));
  }
  const tint_symbol_2 tint_symbol_8 = {output.texcoords, output.position};
  return tint_symbol_8;
}

SamplerState mySampler : register(s1, space0);
Texture2D<float4> myTexture : register(t2, space0);

struct tint_symbol_4 {
  float2 texcoord : TEXCOORD0;
};
struct tint_symbol_5 {
  float4 value : SV_Target0;
};

tint_symbol_5 fs_main(tint_symbol_4 tint_symbol_3) {
  const float2 texcoord = tint_symbol_3.texcoord;
  float2 clampedTexcoord = clamp(texcoord, float2(0.0f, 0.0f), float2(1.0f, 1.0f));
  if (!(all((clampedTexcoord == texcoord)))) {
    discard;
  }
  float4 srcColor = myTexture.Sample(mySampler, texcoord);
  const tint_symbol_5 tint_symbol_9 = {srcColor};
  return tint_symbol_9;
}
