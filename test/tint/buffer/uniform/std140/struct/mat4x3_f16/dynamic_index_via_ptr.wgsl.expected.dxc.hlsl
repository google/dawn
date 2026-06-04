struct Inner {
  matrix<float16_t, 4, 3> m;
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

matrix<float16_t, 4, 3> v_2(uint start_byte_offset) {
  uint4 v_3 = a[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_4 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)).xyz;
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = a[(v_5 / 16u)];
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)).xyz;
  uint v_8 = (16u + start_byte_offset);
  uint4 v_9 = a[(v_8 / 16u)];
  vector<float16_t, 3> v_10 = tint_bitcast_to_f16(select((((v_8 & 15u) >> 2u) == 2u), v_9.zw, v_9.xy)).xyz;
  uint v_11 = (24u + start_byte_offset);
  uint4 v_12 = a[(v_11 / 16u)];
  return matrix<float16_t, 4, 3>(v_4, v_7, v_10, tint_bitcast_to_f16(select((((v_11 & 15u) >> 2u) == 2u), v_12.zw, v_12.xy)).xyz);
}

Inner v_13(uint start_byte_offset) {
  Inner v_14 = {v_2(start_byte_offset)};
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
    }
  }
  Outer v_27[4] = a_1;
  return v_27;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_28 = (min(uint(i()), 3u) * 256u);
  uint v_29 = (min(uint(i()), 3u) * 64u);
  uint v_30 = (min(uint(i()), 3u) * 8u);
  Outer l_a[4] = v_23(0u);
  Outer l_a_i = v_20(v_28);
  Inner l_a_i_a[4] = v_15(v_28);
  Inner l_a_i_a_i = v_13((v_28 + v_29));
  matrix<float16_t, 4, 3> l_a_i_a_i_m = v_2((v_28 + v_29));
  uint v_31 = ((v_28 + v_29) + v_30);
  uint4 v_32 = a[(v_31 / 16u)];
  vector<float16_t, 3> l_a_i_a_i_m_i = tint_bitcast_to_f16(select((((v_31 & 15u) >> 2u) == 2u), v_32.zw, v_32.xy)).xyz;
  uint v_33 = (((v_28 + v_29) + v_30) + (min(uint(i()), 2u) * 2u));
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_f16_1(a[(v_33 / 16u)][((v_33 & 15u) >> 2u)])[select(((v_33 % 4u) == 0u), 0u, 1u)];
}

