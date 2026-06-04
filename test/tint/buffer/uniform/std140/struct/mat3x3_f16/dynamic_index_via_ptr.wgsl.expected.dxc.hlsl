struct Inner {
  matrix<float16_t, 3, 3> m;
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
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 3, 3> v_2(uint start_byte_offset) {
  uint4 v_3 = a[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_4 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)).xyz;
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = a[(v_5 / 16u)];
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)).xyz;
  uint v_8 = (16u + start_byte_offset);
  uint4 v_9 = a[(v_8 / 16u)];
  return matrix<float16_t, 3, 3>(v_4, v_7, tint_bitcast_to_f16(select((((v_8 & 15u) >> 2u) == 2u), v_9.zw, v_9.xy)).xyz);
}

Inner v_10(uint start_byte_offset) {
  Inner v_11 = {v_2(start_byte_offset)};
  return v_11;
}

typedef Inner ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      Inner v_15 = v_10((start_byte_offset + (v_14 * 64u)));
      a_2[v_14] = v_15;
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
  Inner v_16[4] = a_2;
  return v_16;
}

Outer v_17(uint start_byte_offset) {
  Inner v_18[4] = v_12(start_byte_offset);
  Outer v_19 = {v_18};
  return v_19;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_20(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_21 = 0u;
    v_21 = 0u;
    while(true) {
      uint v_22 = v_21;
      if ((v_22 >= 4u)) {
        break;
      }
      Outer v_23 = v_17((start_byte_offset + (v_22 * 256u)));
      a_1[v_22] = v_23;
      {
        v_21 = (v_22 + 1u);
      }
    }
  }
  Outer v_24[4] = a_1;
  return v_24;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_25 = (min(uint(i()), 3u) * 256u);
  uint v_26 = (min(uint(i()), 3u) * 64u);
  uint v_27 = (min(uint(i()), 2u) * 8u);
  Outer l_a[4] = v_20(0u);
  Outer l_a_i = v_17(v_25);
  Inner l_a_i_a[4] = v_12(v_25);
  Inner l_a_i_a_i = v_10((v_25 + v_26));
  matrix<float16_t, 3, 3> l_a_i_a_i_m = v_2((v_25 + v_26));
  uint v_28 = ((v_25 + v_26) + v_27);
  uint4 v_29 = a[(v_28 / 16u)];
  vector<float16_t, 3> l_a_i_a_i_m_i = tint_bitcast_to_f16(select((((v_28 & 15u) >> 2u) == 2u), v_29.zw, v_29.xy)).xyz;
  uint v_30 = (((v_25 + v_26) + v_27) + (min(uint(i()), 2u) * 2u));
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_f16_1(a[(v_30 / 16u)][((v_30 & 15u) >> 2u)])[select(((v_30 % 4u) == 0u), 0u, 1u)];
}

