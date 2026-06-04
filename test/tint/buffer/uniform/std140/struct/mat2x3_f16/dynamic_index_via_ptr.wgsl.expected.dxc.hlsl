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
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 3> v_2(uint start_byte_offset) {
  uint4 v_3 = a[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_4 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)).xyz;
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = a[(v_5 / 16u)];
  return matrix<float16_t, 2, 3>(v_4, tint_bitcast_to_f16(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)).xyz);
}

Inner v_7(uint start_byte_offset) {
  Inner v_8 = {v_2(start_byte_offset)};
  return v_8;
}

typedef Inner ary_ret[4];
ary_ret v_9(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      Inner v_12 = v_7((start_byte_offset + (v_11 * 64u)));
      a_2[v_11] = v_12;
      {
        v_10 = (v_11 + 1u);
      }
    }
  }
  Inner v_13[4] = a_2;
  return v_13;
}

Outer v_14(uint start_byte_offset) {
  Inner v_15[4] = v_9(start_byte_offset);
  Outer v_16 = {v_15};
  return v_16;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_17(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 4u)) {
        break;
      }
      Outer v_20 = v_14((start_byte_offset + (v_19 * 256u)));
      a_1[v_19] = v_20;
      {
        v_18 = (v_19 + 1u);
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
  uint v_24 = (min(uint(i()), 1u) * 8u);
  Outer l_a[4] = v_17(0u);
  Outer l_a_i = v_14(v_22);
  Inner l_a_i_a[4] = v_9(v_22);
  Inner l_a_i_a_i = v_7((v_22 + v_23));
  matrix<float16_t, 2, 3> l_a_i_a_i_m = v_2((v_22 + v_23));
  uint v_25 = ((v_22 + v_23) + v_24);
  uint4 v_26 = a[(v_25 / 16u)];
  vector<float16_t, 3> l_a_i_a_i_m_i = tint_bitcast_to_f16(select((((v_25 & 15u) >> 2u) == 2u), v_26.zw, v_26.xy)).xyz;
  uint v_27 = (((v_22 + v_23) + v_24) + (min(uint(i()), 2u) * 2u));
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_f16_1(a[(v_27 / 16u)][((v_27 & 15u) >> 2u)])[select(((v_27 % 4u) == 0u), 0u, 1u)];
}

