cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[12];
};
RWByteAddressBuffer s : register(u1, space0);

void tint_symbol_1(RWByteAddressBuffer buffer, uint offset, float3x3 value) {
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
  buffer.Store3((offset + 32u), asuint(value[2u]));
}

void tint_symbol(RWByteAddressBuffer buffer, uint offset, float3x3 value[4]) {
  float3x3 array[4] = value;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      tint_symbol_1(buffer, (offset + (i * 48u)), array[i]);
    }
  }
}

float3x3 tint_symbol_4(uint4 buffer[12], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz), asfloat(buffer[scalar_offset_2 / 4].xyz));
}

typedef float3x3 tint_symbol_3_ret[4];
tint_symbol_3_ret tint_symbol_3(uint4 buffer[12], uint offset) {
  float3x3 arr[4] = (float3x3[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = tint_symbol_4(buffer, (offset + (i_1 * 48u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol(s, 0u, tint_symbol_3(u, 0u));
  tint_symbol_1(s, 48u, tint_symbol_4(u, 96u));
  s.Store3(48u, asuint(asfloat(u[1].xyz).zxy));
  s.Store(48u, asuint(asfloat(u[1].x)));
  return;
}
