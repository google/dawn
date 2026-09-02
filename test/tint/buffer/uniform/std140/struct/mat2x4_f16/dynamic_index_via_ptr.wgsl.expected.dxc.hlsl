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

matrix<float16_t, 2, 4> v_1(uint start_byte_offset) {
  uint4 v_2 = a[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_3 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy));
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = a[(v_4 / 16u)];
  return matrix<float16_t, 2, 4>(v_3, tint_bitcast_to_f16(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)));
}

Inner v_6(uint start_byte_offset) {
  Inner v_7 = {v_1(start_byte_offset)};
  return v_7;
}

typedef Inner ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      Inner v_11 = v_6((start_byte_offset + (v_10 * 64u)));
      a_2[v_10] = v_11;
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
  Inner v_12[4] = a_2;
  return v_12;
}

Outer v_13(uint start_byte_offset) {
  Inner v_14[4] = v_8(start_byte_offset);
  Outer v_15 = {v_14};
  return v_15;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_16(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_17 = 0u;
    v_17 = 0u;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 4u)) {
        break;
      }
      Outer v_19 = v_13((start_byte_offset + (v_18 * 256u)));
      a_1[v_18] = v_19;
      {
        v_17 = (v_18 + 1u);
      }
    }
  }
  Outer v_20[4] = a_1;
  return v_20;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_21 = (min(uint(i()), 3u) * 256u);
  uint v_22 = (min(uint(i()), 3u) * 64u);
  uint v_23 = (min(uint(i()), 1u) * 8u);
  Outer l_a[4] = v_16(0u);
  Outer l_a_i = v_13(v_21);
  Inner l_a_i_a[4] = v_8(v_21);
  Inner l_a_i_a_i = v_6((v_21 + v_22));
  matrix<float16_t, 2, 4> l_a_i_a_i_m = v_1((v_21 + v_22));
  uint v_24 = ((v_21 + v_22) + v_23);
  uint4 v_25 = a[(v_24 / 16u)];
  vector<float16_t, 4> l_a_i_a_i_m_i = tint_bitcast_to_f16(select((((v_24 & 15u) >> 2u) == 2u), v_25.zw, v_25.xy));
  uint v_26 = (((v_21 + v_22) + v_23) + (min(uint(i()), 3u) * 2u));
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_f16_1(a[(v_26 / 16u)][((v_26 & 15u) >> 2u)])[select(((v_26 % 4u) == 0u), 0u, 1u)];
}

