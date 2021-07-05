float2x2 tint_symbol_4(uint4 buffer[1], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

float2x2 tint_symbol_6(uint4 buffer[1], uint offset) {
  const uint scalar_offset_2 = ((offset + 0u)) / 4;
  uint4 ubo_load_2 = buffer[scalar_offset_2 / 4];
  const uint scalar_offset_3 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_3 / 4];
  return float2x2(asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy)), asfloat(((scalar_offset_3 & 2) ? ubo_load_3.zw : ubo_load_3.xy)));
}

cbuffer cbuffer_x_20 : register(b0, space0) {
  uint4 x_20[1];
};
cbuffer cbuffer_x_26 : register(b0, space1) {
  uint4 x_26[1];
};

struct tint_symbol_1 {
  uint gl_VertexIndex : SV_VertexID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const uint gl_VertexIndex = tint_symbol.gl_VertexIndex;
  float2 indexable[3] = (float2[3])0;
  const float2x2 x_23 = tint_symbol_4(x_20, 0u);
  const float2x2 x_28 = tint_symbol_6(x_26, 0u);
  const uint x_46 = gl_VertexIndex;
  const float2 tint_symbol_7[3] = {float2(-1.0f, 1.0f), float2(1.0f, 1.0f), float2(-1.0f, -1.0f)};
  indexable = tint_symbol_7;
  const float2 x_51 = indexable[x_46];
  const float2 x_52 = mul(x_51, float2x2((x_23[0u] + x_28[0u]), (x_23[1u] + x_28[1u])));
  const tint_symbol_2 tint_symbol_8 = {float4(x_52.x, x_52.y, 0.0f, 1.0f)};
  return tint_symbol_8;
}
