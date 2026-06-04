
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
void a(matrix<float16_t, 4, 2> m) {
}

void b(vector<float16_t, 2> v) {
}

void c(float16_t f_1) {
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(u[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  vector<float16_t, 2> v_7 = tint_bitcast_to_f16(u[(v_6 / 16u)][((v_6 & 15u) >> 2u)]);
  uint v_8 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_3, v_5, v_7, tint_bitcast_to_f16(u[(v_8 / 16u)][((v_8 & 15u) >> 2u)]));
}

[numthreads(1, 1, 1)]
void f() {
  a(v_2(0u));
  b(tint_bitcast_to_f16(u[0u].y));
  b(tint_bitcast_to_f16(u[0u].y).yx);
  c(tint_bitcast_to_f16(u[0u].y).x);
  c(tint_bitcast_to_f16(u[0u].y).yx.x);
}

