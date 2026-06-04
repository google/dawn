
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
void a(matrix<float16_t, 2, 4> m) {
}

void b(vector<float16_t, 4> v) {
}

void c(float16_t f_1) {
}

vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

vector<float16_t, 2> tint_bitcast_to_f16_1(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 4> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_4 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy));
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = u[(v_5 / 16u)];
  return matrix<float16_t, 2, 4>(v_4, tint_bitcast_to_f16(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)));
}

[numthreads(1, 1, 1)]
void f() {
  a(v_2(0u));
  b(tint_bitcast_to_f16(u[0u].zw));
  b(tint_bitcast_to_f16(u[0u].zw).ywxz);
  c(tint_bitcast_to_f16_1(u[0u].z).x);
  c(tint_bitcast_to_f16(u[0u].zw).ywxz.x);
}

