
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
static matrix<float16_t, 4, 2> p = matrix<float16_t, 4, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 2> v_1(uint start_byte_offset) {
  vector<float16_t, 2> v_2 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(u[(v_3 / 16u)][((v_3 & 15u) >> 2u)]);
  uint v_5 = (8u + start_byte_offset);
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16(u[(v_5 / 16u)][((v_5 & 15u) >> 2u)]);
  uint v_7 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_2, v_4, v_6, tint_bitcast_to_f16(u[(v_7 / 16u)][((v_7 & 15u) >> 2u)]));
}

[numthreads(1, 1, 1)]
void f() {
  p = v_1(0u);
  p[int(1)] = tint_bitcast_to_f16(u[0u].x);
  p[int(1)] = tint_bitcast_to_f16(u[0u].x).yx;
  p[int(0)].y = tint_bitcast_to_f16(u[0u].y).x;
}

