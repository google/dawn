
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_2 = float16_t(t_low.x);
  float16_t v_3 = float16_t(t_high.x);
  float16_t v_4 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_2, v_3, v_4, float16_t(t_high.y));
}

void v_5(uint offset, matrix<float16_t, 2, 3> obj) {
  s.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
}

matrix<float16_t, 2, 3> v_6(uint start_byte_offset) {
  uint4 v_7 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_8 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy))).xyz;
  uint4 v_9 = u[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 3>(v_8, tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_9.zw) : (v_9.xy))).xyz);
}

[numthreads(1, 1, 1)]
void f() {
  v_5(0u, v_6(0u));
  s.Store<vector<float16_t, 3> >(8u, tint_bitcast_to_f16_1(u[0u].xy).xyz);
  s.Store<vector<float16_t, 3> >(8u, tint_bitcast_to_f16_1(u[0u].xy).xyz.zxy);
  s.Store<float16_t>(2u, tint_bitcast_to_f16(u[0u].z).x);
}

