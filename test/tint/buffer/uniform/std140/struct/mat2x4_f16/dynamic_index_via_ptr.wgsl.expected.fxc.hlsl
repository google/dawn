SKIP: INVALID

struct Inner {
  matrix<float16_t, 2, 4> m;
};

struct Outer {
  Inner a[4];
};


cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
};
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
}

vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_1 = float16_t(t_low.x);
  float16_t v_2 = float16_t(t_high.x);
  float16_t v_3 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_1, v_2, v_3, float16_t(t_high.y));
}

matrix<float16_t, 2, 4> v_4(uint start_byte_offset) {
  uint4 v_5 = a[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy)));
  uint4 v_7 = a[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 4>(v_6, tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy))));
}

Inner v_8(uint start_byte_offset) {
  Inner v_9 = {v_4(start_byte_offset)};
  return v_9;
}

typedef Inner ary_ret[4];
ary_ret v_10(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      Inner v_13 = v_8((start_byte_offset + (v_12 * 64u)));
      a_2[v_12] = v_13;
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  Inner v_14[4] = a_2;
  return v_14;
}

Outer v_15(uint start_byte_offset) {
  Inner v_16[4] = v_10(start_byte_offset);
  Outer v_17 = {v_16};
  return v_17;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_18(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_19 = 0u;
    v_19 = 0u;
    while(true) {
      uint v_20 = v_19;
      if ((v_20 >= 4u)) {
        break;
      }
      Outer v_21 = v_15((start_byte_offset + (v_20 * 256u)));
      a_1[v_20] = v_21;
      {
        v_19 = (v_20 + 1u);
      }
      continue;
    }
  }
  Outer v_22[4] = a_1;
  return v_22;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_23 = (256u * min(uint(i()), 3u));
  uint v_24 = (64u * min(uint(i()), 3u));
  uint v_25 = (8u * min(uint(i()), 1u));
  Outer l_a[4] = v_18(0u);
  Outer l_a_i = v_15(v_23);
  Inner l_a_i_a[4] = v_10(v_23);
  Inner l_a_i_a_i = v_8((v_23 + v_24));
  matrix<float16_t, 2, 4> l_a_i_a_i_m = v_4((v_23 + v_24));
  uint4 v_26 = a[(((v_23 + v_24) + v_25) / 16u)];
  vector<float16_t, 4> l_a_i_a_i_m_i = tint_bitcast_to_f16((((((((v_23 + v_24) + v_25) % 16u) / 4u) == 2u)) ? (v_26.zw) : (v_26.xy)));
  uint v_27 = (((v_23 + v_24) + v_25) + (min(uint(i()), 3u) * 2u));
  uint v_28 = a[(v_27 / 16u)][((v_27 % 16u) / 4u)];
  float16_t l_a_i_a_i_m_i_i = float16_t(f16tof32((v_28 >> ((((v_27 % 4u) == 0u)) ? (0u) : (16u)))));
}

