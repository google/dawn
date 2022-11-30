cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[2];
};
RWByteAddressBuffer s : register(u1, space0);

void tint_symbol(RWByteAddressBuffer buffer, uint offset, float2x4 value) {
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
}

float2x4 tint_symbol_2(uint4 buffer[2], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]));
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol(s, 0u, tint_symbol_2(u, 0u));
  s.Store4(16u, asuint(asfloat(u[0])));
  s.Store4(16u, asuint(asfloat(u[0]).ywxz));
  s.Store(4u, asuint(asfloat(u[1].x)));
  return;
}
