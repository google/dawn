cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1, space0);

void tint_symbol(RWByteAddressBuffer buffer, uint offset, float2x2 value) {
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
}

float2x2 tint_symbol_2(uint4 buffer[1], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol(s, 0u, tint_symbol_2(u, 0u));
  s.Store2(8u, asuint(asfloat(u[0].xy)));
  s.Store2(8u, asuint(asfloat(u[0].xy).yx));
  s.Store(4u, asuint(asfloat(u[0].z)));
  return;
}
