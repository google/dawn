struct Inner {
  matrix<float16_t, 3, 4> m;
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

matrix<float16_t, 3, 4> v_5(uint start_byte_offset) {
  uint4 v_6 = a[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_7 = tint_bitcast_to_f16((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_6.zw) : (v_6.xy)));
  uint4 v_8 = a[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_9 = tint_bitcast_to_f16(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_8.zw) : (v_8.xy)));
  uint4 v_10 = a[((16u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 4>(v_7, v_9, tint_bitcast_to_f16(((((((16u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_10.zw) : (v_10.xy))));
}

Inner v_11(uint start_byte_offset) {
  Inner v_12 = {v_5(start_byte_offset)};
  return v_12;
}

typedef Inner ary_ret[4];
ary_ret v_13(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      Inner v_16 = v_11((start_byte_offset + (v_15 * 64u)));
      a_2[v_15] = v_16;
      {
        v_14 = (v_15 + 1u);
      }
      continue;
    }
  }
  Inner v_17[4] = a_2;
  return v_17;
}

Outer v_18(uint start_byte_offset) {
  Inner v_19[4] = v_13(start_byte_offset);
  Outer v_20 = {v_19};
  return v_20;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_21(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_22 = 0u;
    v_22 = 0u;
    while(true) {
      uint v_23 = v_22;
      if ((v_23 >= 4u)) {
        break;
      }
      Outer v_24 = v_18((start_byte_offset + (v_23 * 256u)));
      a_1[v_23] = v_24;
      {
        v_22 = (v_23 + 1u);
      }
      continue;
    }
  }
  Outer v_25[4] = a_1;
  return v_25;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_26 = (256u * min(uint(i()), 3u));
  uint v_27 = (64u * min(uint(i()), 3u));
  uint v_28 = (8u * min(uint(i()), 2u));
  Outer l_a[4] = v_21(0u);
  Outer l_a_i = v_18(v_26);
  Inner l_a_i_a[4] = v_13(v_26);
  Inner l_a_i_a_i = v_11((v_26 + v_27));
  matrix<float16_t, 3, 4> l_a_i_a_i_m = v_5((v_26 + v_27));
  uint4 v_29 = a[(((v_26 + v_27) + v_28) / 16u)];
  vector<float16_t, 4> l_a_i_a_i_m_i = tint_bitcast_to_f16((((((((v_26 + v_27) + v_28) & 15u) >> 2u) == 2u)) ? (v_29.zw) : (v_29.xy)));
  uint v_30 = (((v_26 + v_27) + v_28) + (min(uint(i()), 3u) * 2u));
  uint v_31 = a[(v_30 / 16u)][((v_30 & 15u) >> 2u)];
  uint v_32 = ((((v_30 % 4u) == 0u)) ? (0u) : (1u));
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_f16_1(v_31)[v_32];
}

