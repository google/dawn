struct Inner {
  float4x2 m;
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

float4x2 v(uint start_byte_offset) {
  uint4 v_1 = a[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = a[(v_3 / 16u)];
  float2 v_5 = asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint v_6 = (16u + start_byte_offset);
  uint4 v_7 = a[(v_6 / 16u)];
  float2 v_8 = asfloat((((((v_6 & 15u) >> 2u) == 2u)) ? (v_7.zw) : (v_7.xy)));
  uint v_9 = (24u + start_byte_offset);
  uint4 v_10 = a[(v_9 / 16u)];
  return float4x2(v_2, v_5, v_8, asfloat((((((v_9 & 15u) >> 2u) == 2u)) ? (v_10.zw) : (v_10.xy))));
}

Inner v_11(uint start_byte_offset) {
  Inner v_12 = {v(start_byte_offset)};
  return v_12;
}

typedef Inner ary_ret[4];
ary_ret v_13(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      Inner v_16 = v_11((start_byte_offset + (v_15 * 64u)));
      a_2[v_15] = v_16;
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
  Inner v_17[4] = a_2;
  return v_17;
}

Outer v_18(uint start_byte_offset) {
  Inner v_19[4] = v_13(start_byte_offset);
  Outer v_20 = {v_19};
  return v_20;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_21(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_22 = 0u;
    v_22 = 0u;
    while(true) {
      uint v_23 = v_22;
      if ((v_23 >= 4u)) {
        break;
      }
      Outer v_24 = v_18((start_byte_offset + (v_23 * 256u)));
      a_1[v_23] = v_24;
      {
        v_22 = (v_23 + 1u);
      }
    }
  }
  Outer v_25[4] = a_1;
  return v_25;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_26 = (min(uint(i()), 3u) * 256u);
  uint v_27 = (min(uint(i()), 3u) * 64u);
  uint v_28 = (min(uint(i()), 3u) * 8u);
  Outer l_a[4] = v_21(0u);
  Outer l_a_i = v_18(v_26);
  Inner l_a_i_a[4] = v_13(v_26);
  Inner l_a_i_a_i = v_11((v_26 + v_27));
  float4x2 l_a_i_a_i_m = v((v_26 + v_27));
  uint v_29 = ((v_26 + v_27) + v_28);
  uint4 v_30 = a[(v_29 / 16u)];
  float2 l_a_i_a_i_m_i = asfloat((((((v_29 & 15u) >> 2u) == 2u)) ? (v_30.zw) : (v_30.xy)));
  uint v_31 = (((v_26 + v_27) + v_28) + (min(uint(i()), 1u) * 4u));
  float l_a_i_a_i_m_i_i = asfloat(a[(v_31 / 16u)][((v_31 & 15u) >> 2u)]);
}

