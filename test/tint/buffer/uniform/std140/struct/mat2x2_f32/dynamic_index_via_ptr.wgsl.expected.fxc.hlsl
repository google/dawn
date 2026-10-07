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
  counter = asint((asuint(counter) + 1u));
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
    uint idx = 0u;
    while(true) {
      uint v_8 = idx;
      if ((v_8 >= 4u)) {
        break;
      }
      Inner v_9 = v_5((start_byte_offset + (v_8 * 64u)));
      a_2[v_8] = v_9;
      {
        idx = (idx + 1u);
      }
    }
  }
  Inner v_10[4] = a_2;
  return v_10;
}

Outer v_11(uint start_byte_offset) {
  Inner v_12[4] = v_7(start_byte_offset);
  Outer v_13 = {v_12};
  return v_13;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_14(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_15 = idx;
      if ((v_15 >= 4u)) {
        break;
      }
      Outer v_16 = v_11((start_byte_offset + (v_15 * 256u)));
      a_1[v_15] = v_16;
      {
        idx = (idx + 1u);
      }
    }
  }
  Outer v_17[4] = a_1;
  return v_17;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_18 = (min(uint(i()), 3u) * 256u);
  uint v_19 = (min(uint(i()), 3u) * 64u);
  uint v_20 = (min(uint(i()), 1u) * 8u);
  Outer l_a[4] = v_14(0u);
  Outer l_a_i = v_11(v_18);
  Inner l_a_i_a[4] = v_7(v_18);
  Inner l_a_i_a_i = v_5((v_18 + v_19));
  float2x2 l_a_i_a_i_m = v((v_18 + v_19));
  uint v_21 = ((v_18 + v_19) + v_20);
  uint4 v_22 = a[(v_21 / 16u)];
  float2 l_a_i_a_i_m_i = asfloat((((((v_21 & 15u) >> 2u) == 2u)) ? (v_22.zw) : (v_22.xy)));
  uint v_23 = (((v_18 + v_19) + v_20) + (min(uint(i()), 1u) * 4u));
  float l_a_i_a_i_m_i_i = asfloat(a[(v_23 / 16u)][((v_23 & 15u) >> 2u)]);
}

