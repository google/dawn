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
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(a[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_3, tint_bitcast_to_f16(a[(v_4 / 16u)][((v_4 & 15u) >> 2u)]));
}

Inner v_5(uint start_byte_offset) {
  Inner v_6 = {v_2(start_byte_offset)};
  return v_6;
}

typedef Inner ary_ret[4];
ary_ret v_7(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      Inner v_10 = v_5((start_byte_offset + (v_9 * 64u)));
      a_2[v_9] = v_10;
      {
        v_8 = (v_9 + 1u);
      }
    }
  }
  Inner v_11[4] = a_2;
  return v_11;
}

Outer v_12(uint start_byte_offset) {
  Inner v_13[4] = v_7(start_byte_offset);
  Outer v_14 = {v_13};
  return v_14;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_15(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_16 = 0u;
    v_16 = 0u;
    while(true) {
      uint v_17 = v_16;
      if ((v_17 >= 4u)) {
        break;
      }
      Outer v_18 = v_12((start_byte_offset + (v_17 * 256u)));
      a_1[v_17] = v_18;
      {
        v_16 = (v_17 + 1u);
      }
    }
  }
  Outer v_19[4] = a_1;
  return v_19;
}

[numthreads(1, 1, 1)]
void f() {
  Outer l_a[4] = v_15(0u);
  Outer l_a_3 = v_12(768u);
  Inner l_a_3_a[4] = v_7(768u);
  Inner l_a_3_a_2 = v_5(896u);
  matrix<float16_t, 2, 2> l_a_3_a_2_m = v_2(896u);
  vector<float16_t, 2> l_a_3_a_2_m_1 = tint_bitcast_to_f16(a[56u].y);
  float16_t l_a_3_a_2_m_1_0 = tint_bitcast_to_f16(a[56u].y).x;
}

