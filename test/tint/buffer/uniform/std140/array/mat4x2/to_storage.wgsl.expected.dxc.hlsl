cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1, space0);

void tint_symbol_1(RWByteAddressBuffer buffer, uint offset, float4x2 value) {
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
  buffer.Store2((offset + 16u), asuint(value[2u]));
  buffer.Store2((offset + 24u), asuint(value[3u]));
}

void tint_symbol(RWByteAddressBuffer buffer, uint offset, float4x2 value[4]) {
  float4x2 array[4] = value;
  {
    [loop] for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      tint_symbol_1(buffer, (offset + (i * 32u)), array[i]);
    }
  }
}

float4x2 tint_symbol_4(uint4 buffer[8], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = buffer[scalar_offset_2 / 4];
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_3 / 4];
  return float4x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy)), asfloat(((scalar_offset_3 & 2) ? ubo_load_3.zw : ubo_load_3.xy)));
}

typedef float4x2 tint_symbol_3_ret[4];
tint_symbol_3_ret tint_symbol_3(uint4 buffer[8], uint offset) {
  float4x2 arr[4] = (float4x2[4])0;
  {
    [loop] for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = tint_symbol_4(buffer, (offset + (i_1 * 32u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol(s, 0u, tint_symbol_3(u, 0u));
  tint_symbol_1(s, 32u, tint_symbol_4(u, 64u));
  s.Store2(32u, asuint(asfloat(u[0].zw).yx));
  s.Store(32u, asuint(asfloat(u[0].z)));
  return;
}
