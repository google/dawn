struct Uniforms {
  float4x4 modelViewProjectionMatrix;
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
struct tint_symbol_4 {
  float4 fragColor : TEXCOORD0;
};
struct tint_symbol_5 {
  float4 value : SV_Target0;
};

ConstantBuffer<Uniforms> uniforms : register(b0, space0);

tint_symbol_2 vtx_main(tint_symbol_1 tint_symbol) {
  const VertexInput input = {tint_symbol.cur_position, tint_symbol.color};
  VertexOutput output = {float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)};
  output.Position = mul(input.cur_position, uniforms.modelViewProjectionMatrix);
  output.vtxFragColor = input.color;
  const tint_symbol_2 tint_symbol_6 = {output.vtxFragColor, output.Position};
  return tint_symbol_6;
}

tint_symbol_5 frag_main(tint_symbol_4 tint_symbol_3) {
  const float4 fragColor = tint_symbol_3.fragColor;
  const tint_symbol_5 tint_symbol_7 = {fragColor};
  return tint_symbol_7;
}

