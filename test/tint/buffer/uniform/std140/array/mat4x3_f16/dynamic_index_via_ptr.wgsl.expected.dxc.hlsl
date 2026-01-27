
cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
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

matrix<float16_t, 4, 3> v_5(uint start_byte_offset) {
  uint4 v_6 = a[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_6.zw) : (v_6.xy))).xyz;
  uint4 v_8 = a[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_9 = tint_bitcast_to_f16(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_8.zw) : (v_8.xy))).xyz;
  uint4 v_10 = a[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_11 = tint_bitcast_to_f16(((((((16u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_10.zw) : (v_10.xy))).xyz;
  uint4 v_12 = a[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 3>(v_7, v_9, v_11, tint_bitcast_to_f16(((((((24u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_12.zw) : (v_12.xy))).xyz);
}

typedef matrix<float16_t, 4, 3> ary_ret[4];
ary_ret v_13(uint start_byte_offset) {
  matrix<float16_t, 4, 3> a_1[4] = (matrix<float16_t, 4, 3>[4])0;
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      a_1[v_15] = v_5((start_byte_offset + (v_15 * 32u)));
      {
        v_14 = (v_15 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 3> v_16[4] = a_1;
  return v_16;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_17 = (min(uint(i()), 3u) * 32u);
  uint v_18 = (min(uint(i()), 3u) * 8u);
  matrix<float16_t, 4, 3> l_a[4] = v_13(0u);
  matrix<float16_t, 4, 3> l_a_i = v_5(v_17);
  uint4 v_19 = a[((v_17 + v_18) / 16u)];
  vector<float16_t, 3> l_a_i_i = tint_bitcast_to_f16(((((((v_17 + v_18) & 15u) >> 2u) == 2u)) ? (v_19.zw) : (v_19.xy))).xyz;
  uint v_20 = a[((v_17 + v_18) / 16u)][(((v_17 + v_18) & 15u) >> 2u)];
  uint v_21 = (((((v_17 + v_18) % 4u) == 0u)) ? (0u) : (1u));
  s.Store<float16_t>(0u, (((tint_bitcast_to_f16_1(v_20)[v_21] + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x));
}

