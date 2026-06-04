struct Inner {
  float3x2 m;
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

float3x2 v(uint start_byte_offset) {
  uint4 v_1 = a[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = a[(v_3 / 16u)];
  float2 v_5 = asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint v_6 = (16u + start_byte_offset);
  uint4 v_7 = a[(v_6 / 16u)];
  return float3x2(v_2, v_5, asfloat((((((v_6 & 15u) >> 2u) == 2u)) ? (v_7.zw) : (v_7.xy))));
}

Inner v_8(uint start_byte_offset) {
  Inner v_9 = {v(start_byte_offset)};
  return v_9;
}

typedef Inner ary_ret[4];
ary_ret v_10(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      Inner v_13 = v_8((start_byte_offset + (v_12 * 64u)));
      a_2[v_12] = v_13;
      {
        v_11 = (v_12 + 1u);
      }
    }
  }
  Inner v_14[4] = a_2;
  return v_14;
}

Outer v_15(uint start_byte_offset) {
  Inner v_16[4] = v_10(start_byte_offset);
  Outer v_17 = {v_16};
  return v_17;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_18(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_19 = 0u;
    v_19 = 0u;
    while(true) {
      uint v_20 = v_19;
      if ((v_20 >= 4u)) {
        break;
      }
      Outer v_21 = v_15((start_byte_offset + (v_20 * 256u)));
      a_1[v_20] = v_21;
      {
        v_19 = (v_20 + 1u);
      }
    }
  }
  Outer v_22[4] = a_1;
  return v_22;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_23 = (min(uint(i()), 3u) * 256u);
  uint v_24 = (min(uint(i()), 3u) * 64u);
  uint v_25 = (min(uint(i()), 2u) * 8u);
  Outer l_a[4] = v_18(0u);
  Outer l_a_i = v_15(v_23);
  Inner l_a_i_a[4] = v_10(v_23);
  Inner l_a_i_a_i = v_8((v_23 + v_24));
  float3x2 l_a_i_a_i_m = v((v_23 + v_24));
  uint v_26 = ((v_23 + v_24) + v_25);
  uint4 v_27 = a[(v_26 / 16u)];
  float2 l_a_i_a_i_m_i = asfloat((((((v_26 & 15u) >> 2u) == 2u)) ? (v_27.zw) : (v_27.xy)));
  uint v_28 = (((v_23 + v_24) + v_25) + (min(uint(i()), 1u) * 4u));
  float l_a_i_a_i_m_i_i = asfloat(a[(v_28 / 16u)][((v_28 & 15u) >> 2u)]);
}

