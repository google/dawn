
cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = asint((asuint(counter) + asuint(int(1))));
  return counter;
}

vector<float16_t, 2> tint_bitcast_to_f16_1(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
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

matrix<float16_t, 2, 3> v_5(uint start_byte_offset) {
  uint4 v_6 = a[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_6.zw) : (v_6.xy))).xyz;
  uint4 v_8 = a[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 3>(v_7, tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_8.zw) : (v_8.xy))).xyz);
}

typedef matrix<float16_t, 2, 3> ary_ret[4];
ary_ret v_9(uint start_byte_offset) {
  matrix<float16_t, 2, 3> a_1[4] = (matrix<float16_t, 2, 3>[4])0;
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      a_1[v_11] = v_5((start_byte_offset + (v_11 * 16u)));
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 2, 3> v_12[4] = a_1;
  return v_12;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_13 = (16u * min(uint(i()), 3u));
  uint v_14 = (8u * min(uint(i()), 1u));
  matrix<float16_t, 2, 3> l_a[4] = v_9(0u);
  matrix<float16_t, 2, 3> l_a_i = v_5(v_13);
  uint4 v_15 = a[((v_13 + v_14) / 16u)];
  vector<float16_t, 3> l_a_i_i = tint_bitcast_to_f16(((((((v_13 + v_14) % 16u) / 4u) == 2u)) ? (v_15.zw) : (v_15.xy))).xyz;
  uint v_16 = a[((v_13 + v_14) / 16u)][(((v_13 + v_14) % 16u) / 4u)];
  uint v_17 = (((((v_13 + v_14) % 4u) == 0u)) ? (0u) : (1u));
  s.Store<float16_t>(0u, (((tint_bitcast_to_f16_1(v_16)[v_17] + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x));
}

