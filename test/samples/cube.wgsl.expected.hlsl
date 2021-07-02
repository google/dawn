float4x4 tint_symbol_7(uint4 buffer[4], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]), asfloat(buffer[scalar_offset_3 / 4]));
}

cbuffer cbuffer_uniforms : register(b0, space0) {
  uint4 uniforms[4];
};

struct VertexInput {
  float4 cur_position;
  float4 color;
};
struct VertexOutput {
  float4 vtxFragColor;
  float4 Position;
};
struct tint_symbol_1 {
  float4 cur_position : TEXCOORD0;
  float4 color : TEXCOORD1;
};
struct tint_symbol_2 {
  float4 vtxFragColor : TEXCOORD0;
  float4 Position : SV_Position;
};

tint_symbol_2 vtx_main(tint_symbol_1 tint_symbol) {
  const VertexInput input = {tint_symbol.cur_position, tint_symbol.color};
  VertexOutput output = (VertexOutput)0;
  output.Position = mul(input.cur_position, tint_symbol_7(uniforms, 0u));
  output.vtxFragColor = input.color;
  const tint_symbol_2 tint_symbol_8 = {output.vtxFragColor, output.Position};
  return tint_symbol_8;
}

struct tint_symbol_4 {
  float4 fragColor : TEXCOORD0;
};
struct tint_symbol_5 {
  float4 value : SV_Target0;
};

tint_symbol_5 frag_main(tint_symbol_4 tint_symbol_3) {
  const float4 fragColor = tint_symbol_3.fragColor;
  const tint_symbol_5 tint_symbol_9 = {fragColor};
  return tint_symbol_9;
}
