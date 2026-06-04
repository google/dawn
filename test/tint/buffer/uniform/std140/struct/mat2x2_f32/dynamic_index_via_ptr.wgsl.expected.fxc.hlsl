struct Inner {
  float2x2 m;
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

float2x2 v(uint start_byte_offset) {
  uint4 v_1 = a[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = a[(v_3 / 16u)];
  return float2x2(v_2, asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy))));
}

Inner v_5(uint start_byte_offset) {
  Inner v_6 = {v(start_byte_offset)};
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
  uint v_20 = (min(uint(i()), 3u) * 256u);
  uint v_21 = (min(uint(i()), 3u) * 64u);
  uint v_22 = (min(uint(i()), 1u) * 8u);
  Outer l_a[4] = v_15(0u);
  Outer l_a_i = v_12(v_20);
  Inner l_a_i_a[4] = v_7(v_20);
  Inner l_a_i_a_i = v_5((v_20 + v_21));
  float2x2 l_a_i_a_i_m = v((v_20 + v_21));
  uint v_23 = ((v_20 + v_21) + v_22);
  uint4 v_24 = a[(v_23 / 16u)];
  float2 l_a_i_a_i_m_i = asfloat((((((v_23 & 15u) >> 2u) == 2u)) ? (v_24.zw) : (v_24.xy)));
  uint v_25 = (((v_20 + v_21) + v_22) + (min(uint(i()), 1u) * 4u));
  float l_a_i_a_i_m_i_i = asfloat(a[(v_25 / 16u)][((v_25 & 15u) >> 2u)]);
}

