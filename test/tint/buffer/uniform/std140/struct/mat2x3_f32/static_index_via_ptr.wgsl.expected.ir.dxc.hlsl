struct Inner {
  float2x3 m;
};

struct Outer {
  Inner a[4];
};


cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
};
float2x3 v(uint start_byte_offset) {
  float3 v_1 = asfloat(a[(start_byte_offset / 16u)].xyz);
  return float2x3(v_1, asfloat(a[((16u + start_byte_offset) / 16u)].xyz));
}

Inner v_2(uint start_byte_offset) {
  Inner v_3 = {v(start_byte_offset)};
  return v_3;
}

typedef Inner ary_ret[4];
ary_ret v_4(uint start_byte_offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      Inner v_7 = v_2((start_byte_offset + (v_6 * 64u)));
      a[v_6] = v_7;
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  Inner v_8[4] = a;
  return v_8;
}

Outer v_9(uint start_byte_offset) {
  Inner v_10[4] = v_4(start_byte_offset);
  Outer v_11 = {v_10};
  return v_11;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_12(uint start_byte_offset) {
  Outer a[4] = (Outer[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      Outer v_15 = v_9((start_byte_offset + (v_14 * 256u)));
      a[v_14] = v_15;
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
  Outer v_16[4] = a;
  return v_16;
}

[numthreads(1, 1, 1)]
void f() {
  Outer l_a[4] = v_12(0u);
  Outer l_a_3 = v_9(768u);
  Inner l_a_3_a[4] = v_4(768u);
  Inner l_a_3_a_2 = v_2(896u);
  float2x3 l_a_3_a_2_m = v(896u);
  float3 l_a_3_a_2_m_1 = asfloat(a[57u].xyz);
  float l_a_3_a_2_m_1_0 = asfloat(a[57u].x);
}

