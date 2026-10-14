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
  counter = asint((asuint(counter) + 1u));
  return counter;
}

vector<float16_t, 2> tint_bitcast_to_f16_1(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 3, 4> v_1(uint start_byte_offset) {
  uint4 v_2 = a[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_3 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy));
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = a[(v_4 / 16u)];
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy));
  uint v_7 = (16u + start_byte_offset);
  uint4 v_8 = a[(v_7 / 16u)];
  return matrix<float16_t, 3, 4>(v_3, v_6, tint_bitcast_to_f16(select((((v_7 & 15u) >> 2u) == 2u), v_8.zw, v_8.xy)));
}

Inner v_9(uint start_byte_offset) {
  Inner v_10 = {v_1(start_byte_offset)};
  return v_10;
}

typedef Inner ary_ret[4];
ary_ret v_11(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_12 = idx;
      if ((v_12 >= 4u)) {
        break;
      }
      Inner v_13 = v_9((start_byte_offset + (v_12 * 64u)));
      a_2[v_12] = v_13;
      {
        idx = (idx + 1u);
      }
    }
  }
  Inner v_14[4] = a_2;
  return v_14;
}

Outer v_15(uint start_byte_offset) {
  Inner v_16[4] = v_11(start_byte_offset);
  Outer v_17 = {v_16};
  return v_17;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_18(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_19 = idx;
      if ((v_19 >= 4u)) {
        break;
      }
      Outer v_20 = v_15((start_byte_offset + (v_19 * 256u)));
      a_1[v_19] = v_20;
      {
        idx = (idx + 1u);
      }
    }
  }
  Outer v_21[4] = a_1;
  return v_21;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_22 = (min(uint(i()), 3u) * 256u);
  uint v_23 = (min(uint(i()), 3u) * 64u);
  uint v_24 = (min(uint(i()), 2u) * 8u);
  Outer l_a[4] = v_18(0u);
  Outer l_a_i = v_15(v_22);
  Inner l_a_i_a[4] = v_11(v_22);
  Inner l_a_i_a_i = v_9((v_22 + v_23));
  matrix<float16_t, 3, 4> l_a_i_a_i_m = v_1((v_22 + v_23));
  uint v_25 = ((v_22 + v_23) + v_24);
  uint4 v_26 = a[(v_25 / 16u)];
  vector<float16_t, 4> l_a_i_a_i_m_i = tint_bitcast_to_f16(select((((v_25 & 15u) >> 2u) == 2u), v_26.zw, v_26.xy));
  uint v_27 = (((v_22 + v_23) + v_24) + (min(uint(i()), 3u) * 2u));
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_f16_1(a[(v_27 / 16u)][((v_27 & 15u) >> 2u)])[select(((v_27 % 4u) == 0u), 0u, 1u)];
}

