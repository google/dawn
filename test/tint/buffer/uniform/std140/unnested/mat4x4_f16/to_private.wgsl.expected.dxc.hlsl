
cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
static matrix<float16_t, 4, 4> p = matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
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

matrix<float16_t, 4, 4> v_5(uint start_byte_offset) {
  uint4 v_6 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_7 = tint_bitcast_to_f16_1((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_6.zw) : (v_6.xy)));
  uint4 v_8 = u[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_9 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_8.zw) : (v_8.xy)));
  uint4 v_10 = u[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_11 = tint_bitcast_to_f16_1(((((((16u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_10.zw) : (v_10.xy)));
  uint4 v_12 = u[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 4>(v_7, v_9, v_11, tint_bitcast_to_f16_1(((((((24u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_12.zw) : (v_12.xy))));
}

[numthreads(1, 1, 1)]
void f() {
  p = v_5(0u);
  p[1u] = tint_bitcast_to_f16_1(u[0u].xy);
  p[1u] = tint_bitcast_to_f16_1(u[0u].xy).ywxz;
  p[0u].y = tint_bitcast_to_f16(u[0u].z).x;
}

