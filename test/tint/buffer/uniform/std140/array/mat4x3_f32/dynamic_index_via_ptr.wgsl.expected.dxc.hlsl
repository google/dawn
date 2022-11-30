cbuffer cbuffer_a : register(b0, space0) {
  uint4 a[16];
};
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

float4x3 tint_symbol_1(uint4 buffer[16], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz), asfloat(buffer[scalar_offset_2 / 4].xyz), asfloat(buffer[scalar_offset_3 / 4].xyz));
}

typedef float4x3 tint_symbol_ret[4];
tint_symbol_ret tint_symbol(uint4 buffer[16], uint offset) {
  float4x3 arr[4] = (float4x3[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = tint_symbol_1(buffer, (offset + (i_1 * 64u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  const int p_a_i_save = i();
  const int p_a_i_i_save = i();
  const float4x3 l_a[4] = tint_symbol(a, 0u);
  const float4x3 l_a_i = tint_symbol_1(a, (64u * uint(p_a_i_save)));
  const uint scalar_offset_4 = (((64u * uint(p_a_i_save)) + (16u * uint(p_a_i_i_save)))) / 4;
  const float3 l_a_i_i = asfloat(a[scalar_offset_4 / 4].xyz);
  return;
}
