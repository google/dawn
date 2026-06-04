
cbuffer cbuffer_m : register(b0) {
  uint4 m[1];
};
static int counter = int(0);
int i() {
  counter = asint((asuint(counter) + asuint(int(1))));
  return counter;
}

vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 3> v_1(uint start_byte_offset) {
  uint4 v_2 = m[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_3 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy)).xyz;
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = m[(v_4 / 16u)];
  return matrix<float16_t, 2, 3>(v_3, tint_bitcast_to_f16(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)).xyz);
}

[numthreads(1, 1, 1)]
void f() {
  uint v_6 = (min(uint(i()), 1u) * 8u);
  matrix<float16_t, 2, 3> l_m = v_1(0u);
  uint4 v_7 = m[(v_6 / 16u)];
  vector<float16_t, 3> l_m_i = tint_bitcast_to_f16(select((((v_6 & 15u) >> 2u) == 2u), v_7.zw, v_7.xy)).xyz;
}

