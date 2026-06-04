
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
static matrix<float16_t, 2, 2> p = matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_3, tint_bitcast_to_f16(u[(v_4 / 16u)][((v_4 & 15u) >> 2u)]));
}

[numthreads(1, 1, 1)]
void f() {
  p = v_2(0u);
  p[1u] = tint_bitcast_to_f16(u[0u].x);
  p[1u] = tint_bitcast_to_f16(u[0u].x).yx;
  p[0u].y = tint_bitcast_to_f16(u[0u].y).x;
}

