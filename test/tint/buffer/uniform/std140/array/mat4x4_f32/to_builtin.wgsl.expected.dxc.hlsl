cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[16];
};

float4x4 tint_symbol(uint4 buffer[16], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]), asfloat(buffer[scalar_offset_3 / 4]));
}

[numthreads(1, 1, 1)]
void f() {
  const float4x4 t = transpose(tint_symbol(u, 128u));
  const float l = length(asfloat(u[1]).ywxz);
  const float a = abs(asfloat(u[1]).ywxz.x);
  return;
}
