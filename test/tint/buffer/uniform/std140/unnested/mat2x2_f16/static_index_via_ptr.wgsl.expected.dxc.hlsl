
cbuffer cbuffer_m : register(b0) {
  uint4 m[1];
};
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(m[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_3, tint_bitcast_to_f16(m[(v_4 / 16u)][((v_4 & 15u) >> 2u)]));
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 2> l_m = v_2(0u);
  vector<float16_t, 2> l_m_1 = tint_bitcast_to_f16(m[0u].y);
}

