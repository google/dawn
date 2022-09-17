cbuffer cbuffer_a : register(b0, space0) {
  uint4 a[8];
};

float4x2 tint_symbol_1(uint4 buffer[8], uint offset) {
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

typedef float4x2 tint_symbol_ret[4];
tint_symbol_ret tint_symbol(uint4 buffer[8], uint offset) {
  float4x2 arr[4] = (float4x2[4])0;
  {
    [loop] for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = tint_symbol_1(buffer, (offset + (i * 32u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  const float4x2 l_a[4] = tint_symbol(a, 0u);
  const float4x2 l_a_i = tint_symbol_1(a, 64u);
  const float2 l_a_i_i = asfloat(a[4].zw);
  return;
}
