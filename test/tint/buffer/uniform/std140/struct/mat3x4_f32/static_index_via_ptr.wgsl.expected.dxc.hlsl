struct Inner {
  float3x4 m;
};

struct Outer {
  Inner a[4];
};


cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
};
float3x4 v(uint start_byte_offset) {
  return float3x4(asfloat(a[(start_byte_offset / 16u)]), asfloat(a[((16u + start_byte_offset) / 16u)]), asfloat(a[((32u + start_byte_offset) / 16u)]));
}

Inner v_1(uint start_byte_offset) {
  Inner v_2 = {v(start_byte_offset)};
  return v_2;
}

typedef Inner ary_ret[4];
ary_ret v_3(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_4 = idx;
      if ((v_4 >= 4u)) {
        break;
      }
      Inner v_5 = v_1((start_byte_offset + (v_4 * 64u)));
      a_2[v_4] = v_5;
      {
        idx = (idx + 1u);
      }
    }
  }
  Inner v_6[4] = a_2;
  return v_6;
}

Outer v_7(uint start_byte_offset) {
  Inner v_8[4] = v_3(start_byte_offset);
  Outer v_9 = {v_8};
  return v_9;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_10(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_11 = idx;
      if ((v_11 >= 4u)) {
        break;
      }
      Outer v_12 = v_7((start_byte_offset + (v_11 * 256u)));
      a_1[v_11] = v_12;
      {
        idx = (idx + 1u);
      }
    }
  }
  Outer v_13[4] = a_1;
  return v_13;
}

[numthreads(1, 1, 1)]
void f() {
  Outer l_a[4] = v_10(0u);
  Outer l_a_3 = v_7(768u);
  Inner l_a_3_a[4] = v_3(768u);
  Inner l_a_3_a_2 = v_1(896u);
  float3x4 l_a_3_a_2_m = v(896u);
  float4 l_a_3_a_2_m_1 = asfloat(a[57u]);
  float l_a_3_a_2_m_1_0 = asfloat(a[57u].x);
}

