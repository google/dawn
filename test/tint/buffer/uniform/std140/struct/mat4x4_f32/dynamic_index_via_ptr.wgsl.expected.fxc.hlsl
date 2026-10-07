struct Inner {
  float4x4 m;
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

float4x4 v(uint start_byte_offset) {
  return float4x4(asfloat(a[(start_byte_offset / 16u)]), asfloat(a[((16u + start_byte_offset) / 16u)]), asfloat(a[((32u + start_byte_offset) / 16u)]), asfloat(a[((48u + start_byte_offset) / 16u)]));
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
  uint v_14 = (min(uint(i()), 3u) * 256u);
  uint v_15 = (min(uint(i()), 3u) * 64u);
  uint v_16 = (min(uint(i()), 3u) * 16u);
  Outer l_a[4] = v_10(0u);
  Outer l_a_i = v_7(v_14);
  Inner l_a_i_a[4] = v_3(v_14);
  Inner l_a_i_a_i = v_1((v_14 + v_15));
  float4x4 l_a_i_a_i_m = v((v_14 + v_15));
  float4 l_a_i_a_i_m_i = asfloat(a[(((v_14 + v_15) + v_16) / 16u)]);
  uint v_17 = (((v_14 + v_15) + v_16) + (min(uint(i()), 3u) * 4u));
  float l_a_i_a_i_m_i_i = asfloat(a[(v_17 / 16u)][((v_17 & 15u) >> 2u)]);
}

