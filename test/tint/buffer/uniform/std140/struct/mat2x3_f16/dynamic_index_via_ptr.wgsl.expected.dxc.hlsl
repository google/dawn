struct Inner {
  matrix<float16_t, 2, 3> m;
};

struct Outer {
  Inner a[4];
};


cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
};
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
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_6.zw) : (v_6.xy))).xyz;
  uint4 v_8 = a[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 3>(v_7, tint_bitcast_to_f16(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_8.zw) : (v_8.xy))).xyz);
}

Inner v_9(uint start_byte_offset) {
  Inner v_10 = {v_5(start_byte_offset)};
  return v_10;
}

typedef Inner ary_ret[4];
ary_ret v_11(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      Inner v_14 = v_9((start_byte_offset + (v_13 * 64u)));
      a_2[v_13] = v_14;
      {
        v_12 = (v_13 + 1u);
      }
      continue;
    }
  }
  Inner v_15[4] = a_2;
  return v_15;
}

Outer v_16(uint start_byte_offset) {
  Inner v_17[4] = v_11(start_byte_offset);
  Outer v_18 = {v_17};
  return v_18;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_19(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_20 = 0u;
    v_20 = 0u;
    while(true) {
      uint v_21 = v_20;
      if ((v_21 >= 4u)) {
        break;
      }
      Outer v_22 = v_16((start_byte_offset + (v_21 * 256u)));
      a_1[v_21] = v_22;
      {
        v_20 = (v_21 + 1u);
      }
      continue;
    }
  }
  Outer v_23[4] = a_1;
  return v_23;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_24 = (min(uint(i()), 3u) * 256u);
  uint v_25 = (min(uint(i()), 3u) * 64u);
  uint v_26 = (min(uint(i()), 1u) * 8u);
  Outer l_a[4] = v_19(0u);
  Outer l_a_i = v_16(v_24);
  Inner l_a_i_a[4] = v_11(v_24);
  Inner l_a_i_a_i = v_9((v_24 + v_25));
  matrix<float16_t, 2, 3> l_a_i_a_i_m = v_5((v_24 + v_25));
  uint4 v_27 = a[(((v_24 + v_25) + v_26) / 16u)];
  vector<float16_t, 3> l_a_i_a_i_m_i = tint_bitcast_to_f16((((((((v_24 + v_25) + v_26) & 15u) >> 2u) == 2u)) ? (v_27.zw) : (v_27.xy))).xyz;
  uint v_28 = (((v_24 + v_25) + v_26) + (min(uint(i()), 2u) * 2u));
  uint v_29 = a[(v_28 / 16u)][((v_28 & 15u) >> 2u)];
  uint v_30 = ((((v_28 % 4u) == 0u)) ? (0u) : (1u));
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_f16_1(v_29)[v_30];
}

