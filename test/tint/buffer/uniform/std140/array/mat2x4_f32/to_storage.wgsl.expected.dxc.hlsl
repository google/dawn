cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1, space0);

void tint_symbol_1(RWByteAddressBuffer buffer, uint offset, float2x4 value) {
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
}

void tint_symbol(RWByteAddressBuffer buffer, uint offset, float2x4 value[4]) {
  float2x4 array_1[4] = value;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      tint_symbol_1(buffer, (offset + (i * 32u)), array_1[i]);
    }
  }
}

float2x4 tint_symbol_4(uint4 buffer[8], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]));
}

typedef float2x4 tint_symbol_3_ret[4];
tint_symbol_3_ret tint_symbol_3(uint4 buffer[8], uint offset) {
  float2x4 arr[4] = (float2x4[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = tint_symbol_4(buffer, (offset + (i_1 * 32u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol(s, 0u, tint_symbol_3(u, 0u));
  tint_symbol_1(s, 32u, tint_symbol_4(u, 64u));
  s.Store4(32u, asuint(asfloat(u[1]).ywxz));
  s.Store(32u, asuint(asfloat(u[1].x)));
  return;
}
