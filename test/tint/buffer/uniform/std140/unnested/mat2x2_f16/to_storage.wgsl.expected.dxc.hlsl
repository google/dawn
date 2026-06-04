
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

void v_2(uint offset, matrix<float16_t, 2, 2> obj) {
  s.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
}

matrix<float16_t, 2, 2> v_3(uint start_byte_offset) {
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_5 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_4, tint_bitcast_to_f16(u[(v_5 / 16u)][((v_5 & 15u) >> 2u)]));
}

[numthreads(1, 1, 1)]
void f() {
  v_2(0u, v_3(0u));
  s.Store<vector<float16_t, 2> >(4u, tint_bitcast_to_f16(u[0u].x));
  s.Store<vector<float16_t, 2> >(4u, tint_bitcast_to_f16(u[0u].x).yx);
  s.Store<float16_t>(2u, tint_bitcast_to_f16(u[0u].y).x);
}

