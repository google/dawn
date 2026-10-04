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

matrix<float16_t, 2, 3> v_1(uint start_byte_offset) {
  uint4 v_2 = a[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_3 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy)).xyz;
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = a[(v_4 / 16u)];
  return matrix<float16_t, 2, 3>(v_3, tint_bitcast_to_f16(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)).xyz);
}

Inner v_6(uint start_byte_offset) {
  Inner v_7 = {v_1(start_byte_offset)};
  return v_7;
}

typedef Inner ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_9 = idx;
      if ((v_9 >= 4u)) {
        break;
      }
      Inner v_10 = v_6((start_byte_offset + (v_9 * 64u)));
      a_2[v_9] = v_10;
      {
        idx = (idx + 1u);
      }
    }
  }
  Inner v_11[4] = a_2;
  return v_11;
}

Outer v_12(uint start_byte_offset) {
  Inner v_13[4] = v_8(start_byte_offset);
  Outer v_14 = {v_13};
  return v_14;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_15(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_16 = idx;
      if ((v_16 >= 4u)) {
        break;
      }
      Outer v_17 = v_12((start_byte_offset + (v_16 * 256u)));
      a_1[v_16] = v_17;
      {
        idx = (idx + 1u);
      }
    }
  }
  Outer v_18[4] = a_1;
  return v_18;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_19 = (min(uint(i()), 3u) * 256u);
  uint v_20 = (min(uint(i()), 3u) * 64u);
  uint v_21 = (min(uint(i()), 1u) * 8u);
  Outer l_a[4] = v_15(0u);
  Outer l_a_i = v_12(v_19);
  Inner l_a_i_a[4] = v_8(v_19);
  Inner l_a_i_a_i = v_6((v_19 + v_20));
  matrix<float16_t, 2, 3> l_a_i_a_i_m = v_1((v_19 + v_20));
  uint v_22 = ((v_19 + v_20) + v_21);
  uint4 v_23 = a[(v_22 / 16u)];
  vector<float16_t, 3> l_a_i_a_i_m_i = tint_bitcast_to_f16(select((((v_22 & 15u) >> 2u) == 2u), v_23.zw, v_23.xy)).xyz;
  uint v_24 = (((v_19 + v_20) + v_21) + (min(uint(i()), 2u) * 2u));
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_f16_1(a[(v_24 / 16u)][((v_24 & 15u) >> 2u)])[select(((v_24 % 4u) == 0u), 0u, 1u)];
}

