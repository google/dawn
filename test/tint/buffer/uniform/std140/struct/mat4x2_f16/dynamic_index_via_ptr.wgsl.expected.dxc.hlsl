struct Inner {
  matrix<float16_t, 4, 2> m;
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

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(a[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(a[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  vector<float16_t, 2> v_7 = tint_bitcast_to_f16(a[(v_6 / 16u)][((v_6 & 15u) >> 2u)]);
  uint v_8 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_3, v_5, v_7, tint_bitcast_to_f16(a[(v_8 / 16u)][((v_8 & 15u) >> 2u)]));
}

Inner v_9(uint start_byte_offset) {
  Inner v_10 = {v_2(start_byte_offset)};
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
    }
  }
  Outer v_23[4] = a_1;
  return v_23;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_24 = (min(uint(i()), 3u) * 256u);
  uint v_25 = (min(uint(i()), 3u) * 64u);
  uint v_26 = (min(uint(i()), 3u) * 4u);
  Outer l_a[4] = v_19(0u);
  Outer l_a_i = v_16(v_24);
  Inner l_a_i_a[4] = v_11(v_24);
  Inner l_a_i_a_i = v_9((v_24 + v_25));
  matrix<float16_t, 4, 2> l_a_i_a_i_m = v_2((v_24 + v_25));
  uint v_27 = ((v_24 + v_25) + v_26);
  vector<float16_t, 2> l_a_i_a_i_m_i = tint_bitcast_to_f16(a[(v_27 / 16u)][((v_27 & 15u) >> 2u)]);
  uint v_28 = (((v_24 + v_25) + v_26) + (min(uint(i()), 1u) * 2u));
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_f16(a[(v_28 / 16u)][((v_28 & 15u) >> 2u)])[select(((v_28 % 4u) == 0u), 0u, 1u)];
}

