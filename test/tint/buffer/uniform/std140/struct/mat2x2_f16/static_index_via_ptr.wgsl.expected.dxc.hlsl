struct Inner {
  matrix<float16_t, 2, 2> m;
};

struct Outer {
  Inner a[4];
};


cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
};
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 2> v_1(uint start_byte_offset) {
  vector<float16_t, 2> v_2 = tint_bitcast_to_f16(a[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_2, tint_bitcast_to_f16(a[(v_3 / 16u)][((v_3 & 15u) >> 2u)]));
}

Inner v_4(uint start_byte_offset) {
  Inner v_5 = {v_1(start_byte_offset)};
  return v_5;
}

typedef Inner ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_7 = idx;
      if ((v_7 >= 4u)) {
        break;
      }
      Inner v_8 = v_4((start_byte_offset + (v_7 * 64u)));
      a_2[v_7] = v_8;
      {
        idx = (idx + 1u);
      }
    }
  }
  Inner v_9[4] = a_2;
  return v_9;
}

Outer v_10(uint start_byte_offset) {
  Inner v_11[4] = v_6(start_byte_offset);
  Outer v_12 = {v_11};
  return v_12;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_13(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_14 = idx;
      if ((v_14 >= 4u)) {
        break;
      }
      Outer v_15 = v_10((start_byte_offset + (v_14 * 256u)));
      a_1[v_14] = v_15;
      {
        idx = (idx + 1u);
      }
    }
  }
  Outer v_16[4] = a_1;
  return v_16;
}

[numthreads(1, 1, 1)]
void f() {
  Outer l_a[4] = v_13(0u);
  Outer l_a_3 = v_10(768u);
  Inner l_a_3_a[4] = v_6(768u);
  Inner l_a_3_a_2 = v_4(896u);
  matrix<float16_t, 2, 2> l_a_3_a_2_m = v_1(896u);
  vector<float16_t, 2> l_a_3_a_2_m_1 = tint_bitcast_to_f16(a[56u].y);
  float16_t l_a_3_a_2_m_1_0 = tint_bitcast_to_f16(a[56u].y).x;
}

