struct Inner {
  float scalar_f32;
  float3 vec3_f32;
  float2x4 mat2x4_f32;
};
struct S {
  Inner inner;
};

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[4];
};

float2x4 tint_symbol_4(uint4 buffer[4], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]));
}

Inner tint_symbol_1(uint4 buffer[4], uint offset) {
  const uint scalar_offset_2 = ((offset + 0u)) / 4;
  const uint scalar_offset_3 = ((offset + 16u)) / 4;
  const Inner tint_symbol_6 = {asfloat(buffer[scalar_offset_2 / 4][scalar_offset_2 % 4]), asfloat(buffer[scalar_offset_3 / 4].xyz), tint_symbol_4(buffer, (offset + 32u))};
  return tint_symbol_6;
}

S tint_symbol(uint4 buffer[4], uint offset) {
  const S tint_symbol_7 = {tint_symbol_1(buffer, (offset + 0u))};
  return tint_symbol_7;
}

[numthreads(1, 1, 1)]
void main() {
  const S x = tint_symbol(u, 0u);
  return;
}
