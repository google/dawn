struct Inner {
  float4x2 m;
};

struct Outer {
  Inner a[4];
};


cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
};
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
    uint idx = 0u;
    while(true) {
      uint v_14 = idx;
      if ((v_14 >= 4u)) {
        break;
      }
      Inner v_15 = v_11((start_byte_offset + (v_14 * 64u)));
      a_2[v_14] = v_15;
      {
        idx = (idx + 1u);
      }
    }
  }
  Inner v_16[4] = a_2;
  return v_16;
}

Outer v_17(uint start_byte_offset) {
  Inner v_18[4] = v_13(start_byte_offset);
  Outer v_19 = {v_18};
  return v_19;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_20(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_21 = idx;
      if ((v_21 >= 4u)) {
        break;
      }
      Outer v_22 = v_17((start_byte_offset + (v_21 * 256u)));
      a_1[v_21] = v_22;
      {
        idx = (idx + 1u);
      }
    }
  }
  Outer v_23[4] = a_1;
  return v_23;
}

[numthreads(1, 1, 1)]
void f() {
  Outer l_a[4] = v_20(0u);
  Outer l_a_3 = v_17(768u);
  Inner l_a_3_a[4] = v_13(768u);
  Inner l_a_3_a_2 = v_11(896u);
  float4x2 l_a_3_a_2_m = v(896u);
  float2 l_a_3_a_2_m_1 = asfloat(a[56u].zw);
  float l_a_3_a_2_m_1_0 = asfloat(a[56u].z);
}

