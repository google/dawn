
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

void v_1(uint offset, matrix<float16_t, 3, 2> obj) {
  s.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  s.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
}

matrix<float16_t, 3, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(u[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  return matrix<float16_t, 3, 2>(v_3, v_5, tint_bitcast_to_f16(u[(v_6 / 16u)][((v_6 & 15u) >> 2u)]));
}

[numthreads(1, 1, 1)]
void f() {
  v_1(0u, v_2(0u));
  s.Store<vector<float16_t, 2> >(4u, tint_bitcast_to_f16(u[0u].x));
  s.Store<vector<float16_t, 2> >(4u, tint_bitcast_to_f16(u[0u].x).yx);
  s.Store<float16_t>(2u, tint_bitcast_to_f16(u[0u].y).x);
}

