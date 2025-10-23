struct Inner {
  matrix<float16_t, 4, 4> m;
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

matrix<float16_t, 4, 4> v_5(uint start_byte_offset) {
  uint4 v_6 = a[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_7 = tint_bitcast_to_f16((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_6.zw) : (v_6.xy)));
  uint4 v_8 = a[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_9 = tint_bitcast_to_f16(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_8.zw) : (v_8.xy)));
  uint4 v_10 = a[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_11 = tint_bitcast_to_f16(((((((16u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_10.zw) : (v_10.xy)));
  uint4 v_12 = a[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 4>(v_7, v_9, v_11, tint_bitcast_to_f16(((((((24u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_12.zw) : (v_12.xy))));
}

Inner v_13(uint start_byte_offset) {
  Inner v_14 = {v_5(start_byte_offset)};
  return v_14;
}

typedef Inner ary_ret[4];
ary_ret v_15(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_16 = 0u;
    v_16 = 0u;
    while(true) {
      uint v_17 = v_16;
      if ((v_17 >= 4u)) {
        break;
      }
      Inner v_18 = v_13((start_byte_offset + (v_17 * 64u)));
      a_2[v_17] = v_18;
      {
        v_16 = (v_17 + 1u);
      }
      continue;
    }
  }
  Inner v_19[4] = a_2;
  return v_19;
}

Outer v_20(uint start_byte_offset) {
  Inner v_21[4] = v_15(start_byte_offset);
  Outer v_22 = {v_21};
  return v_22;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_23(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_24 = 0u;
    v_24 = 0u;
    while(true) {
      uint v_25 = v_24;
      if ((v_25 >= 4u)) {
        break;
      }
      Outer v_26 = v_20((start_byte_offset + (v_25 * 256u)));
      a_1[v_25] = v_26;
      {
        v_24 = (v_25 + 1u);
      }
      continue;
    }
  }
  Outer v_27[4] = a_1;
  return v_27;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_28 = (256u * min(uint(i()), 3u));
  uint v_29 = (64u * min(uint(i()), 3u));
  uint v_30 = (8u * min(uint(i()), 3u));
  Outer l_a[4] = v_23(0u);
  Outer l_a_i = v_20(v_28);
  Inner l_a_i_a[4] = v_15(v_28);
  Inner l_a_i_a_i = v_13((v_28 + v_29));
  matrix<float16_t, 4, 4> l_a_i_a_i_m = v_5((v_28 + v_29));
  uint4 v_31 = a[(((v_28 + v_29) + v_30) / 16u)];
  vector<float16_t, 4> l_a_i_a_i_m_i = tint_bitcast_to_f16((((((((v_28 + v_29) + v_30) & 15u) >> 2u) == 2u)) ? (v_31.zw) : (v_31.xy)));
  uint v_32 = (((v_28 + v_29) + v_30) + (min(uint(i()), 3u) * 2u));
  uint v_33 = a[(v_32 / 16u)][((v_32 & 15u) >> 2u)];
  uint v_34 = ((((v_32 % 4u) == 0u)) ? (0u) : (1u));
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_f16_1(v_33)[v_34];
}

