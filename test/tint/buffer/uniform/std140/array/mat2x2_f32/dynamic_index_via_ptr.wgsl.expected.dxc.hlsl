cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

float2x2 a_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = a[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = a[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

typedef float2x2 a_load_ret[4];
a_load_ret a_load(uint offset) {
  float2x2 arr[4] = (float2x2[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = a_load_1((offset + (i_1 * 16u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  const int p_a_i_save = i();
  const int p_a_i_i_save = i();
  float2x2 l_a[4] = a_load(0u);
  const float2x2 l_a_i = a_load_1((16u * uint(p_a_i_save)));
  const uint scalar_offset_2 = (((16u * uint(p_a_i_save)) + (8u * uint(p_a_i_i_save)))) / 4;
  uint4 ubo_load_2 = a[scalar_offset_2 / 4];
  const float2 l_a_i_i = asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy));
  return;
}
