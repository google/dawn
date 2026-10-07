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
  counter = asint((asuint(counter) + 1u));
  return counter;
}

float3x2 v(uint start_byte_offset) {
  uint4 v_1 = a[(start_byte_offset / 16u)];
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = a[(v_2 / 16u)];
  uint v_4 = (16u + start_byte_offset);
  uint4 v_5 = a[(v_4 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)), asfloat(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)));
}

Inner v_6(uint start_byte_offset) {
  Inner v_7 = {v(start_byte_offset)};
  return v_7;
}

typedef Inner ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_9 = idx;
      if ((v_9 >= 4u)) {
        break;
      }
      Inner v_10 = v_6((start_byte_offset + (v_9 * 64u)));
      a_2[v_9] = v_10;
      {
        idx = (idx + 1u);
      }
    }
  }
  Inner v_11[4] = a_2;
  return v_11;
}

Outer v_12(uint start_byte_offset) {
  Inner v_13[4] = v_8(start_byte_offset);
  Outer v_14 = {v_13};
  return v_14;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_15(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_16 = idx;
      if ((v_16 >= 4u)) {
        break;
      }
      Outer v_17 = v_12((start_byte_offset + (v_16 * 256u)));
      a_1[v_16] = v_17;
      {
        idx = (idx + 1u);
      }
    }
  }
  Outer v_18[4] = a_1;
  return v_18;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_19 = (min(uint(i()), 3u) * 256u);
  uint v_20 = (min(uint(i()), 3u) * 64u);
  uint v_21 = (min(uint(i()), 2u) * 8u);
  Outer l_a[4] = v_15(0u);
  Outer l_a_i = v_12(v_19);
  Inner l_a_i_a[4] = v_8(v_19);
  Inner l_a_i_a_i = v_6((v_19 + v_20));
  float3x2 l_a_i_a_i_m = v((v_19 + v_20));
  uint v_22 = ((v_19 + v_20) + v_21);
  uint4 v_23 = a[(v_22 / 16u)];
  float2 l_a_i_a_i_m_i = asfloat(select((((v_22 & 15u) >> 2u) == 2u), v_23.zw, v_23.xy));
  uint v_24 = (((v_19 + v_20) + v_21) + (min(uint(i()), 1u) * 4u));
  float l_a_i_a_i_m_i_i = asfloat(a[(v_24 / 16u)][((v_24 & 15u) >> 2u)]);
}

