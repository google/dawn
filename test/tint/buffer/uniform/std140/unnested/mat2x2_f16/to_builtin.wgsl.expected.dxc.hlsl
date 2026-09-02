
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 2> v_1(uint start_byte_offset) {
  vector<float16_t, 2> v_2 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_2, tint_bitcast_to_f16(u[(v_3 / 16u)][((v_3 & 15u) >> 2u)]));
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 2> t = transpose(v_1(0u));
  float16_t l = length(tint_bitcast_to_f16(u[0u].y));
  float16_t a = abs(tint_bitcast_to_f16(u[0u].x).y);
}

