
cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
static matrix<float16_t, 4, 3> p = matrix<float16_t, 4, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
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

matrix<float16_t, 4, 3> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_4 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)).xyz;
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = u[(v_5 / 16u)];
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16_1(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)).xyz;
  uint v_8 = (16u + start_byte_offset);
  uint4 v_9 = u[(v_8 / 16u)];
  vector<float16_t, 3> v_10 = tint_bitcast_to_f16_1(select((((v_8 & 15u) >> 2u) == 2u), v_9.zw, v_9.xy)).xyz;
  uint v_11 = (24u + start_byte_offset);
  uint4 v_12 = u[(v_11 / 16u)];
  return matrix<float16_t, 4, 3>(v_4, v_7, v_10, tint_bitcast_to_f16_1(select((((v_11 & 15u) >> 2u) == 2u), v_12.zw, v_12.xy)).xyz);
}

[numthreads(1, 1, 1)]
void f() {
  p = v_2(0u);
  p[1u] = tint_bitcast_to_f16_1(u[0u].xy).xyz;
  p[1u] = tint_bitcast_to_f16_1(u[0u].xy).xyz.zxy;
  p[0u].y = tint_bitcast_to_f16(u[0u].z).x;
}

