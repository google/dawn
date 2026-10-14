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
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = a[(v_2 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)));
}

Inner v_4(uint start_byte_offset) {
  Inner v_5 = {v(start_byte_offset)};
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
  uint v_17 = (min(uint(i()), 3u) * 256u);
  uint v_18 = (min(uint(i()), 3u) * 64u);
  uint v_19 = (min(uint(i()), 1u) * 8u);
  Outer l_a[4] = v_13(0u);
  Outer l_a_i = v_10(v_17);
  Inner l_a_i_a[4] = v_6(v_17);
  Inner l_a_i_a_i = v_4((v_17 + v_18));
  float2x2 l_a_i_a_i_m = v((v_17 + v_18));
  uint v_20 = ((v_17 + v_18) + v_19);
  uint4 v_21 = a[(v_20 / 16u)];
  float2 l_a_i_a_i_m_i = asfloat(select((((v_20 & 15u) >> 2u) == 2u), v_21.zw, v_21.xy));
  uint v_22 = (((v_17 + v_18) + v_19) + (min(uint(i()), 1u) * 4u));
  float l_a_i_a_i_m_i_i = asfloat(a[(v_22 / 16u)][((v_22 & 15u) >> 2u)]);
}

