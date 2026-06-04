
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

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

void v_2(uint offset, matrix<float16_t, 2, 3> obj) {
  s.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
}

matrix<float16_t, 2, 3> v_3(uint start_byte_offset) {
  uint4 v_4 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_5 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_4.zw, v_4.xy)).xyz;
  uint v_6 = (8u + start_byte_offset);
  uint4 v_7 = u[(v_6 / 16u)];
  return matrix<float16_t, 2, 3>(v_5, tint_bitcast_to_f16_1(select((((v_6 & 15u) >> 2u) == 2u), v_7.zw, v_7.xy)).xyz);
}

[numthreads(1, 1, 1)]
void f() {
  v_2(0u, v_3(0u));
  s.Store<vector<float16_t, 3> >(8u, tint_bitcast_to_f16_1(u[0u].xy).xyz);
  s.Store<vector<float16_t, 3> >(8u, tint_bitcast_to_f16_1(u[0u].xy).xyz.zxy);
  s.Store<float16_t>(2u, tint_bitcast_to_f16(u[0u].z).x);
}

