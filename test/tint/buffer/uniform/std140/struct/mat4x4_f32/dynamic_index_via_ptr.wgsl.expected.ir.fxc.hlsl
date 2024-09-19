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
  counter = (counter + int(1));
  return counter;
}

float4x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(a[(start_byte_offset / 16u)]);
  float4 v_2 = asfloat(a[((16u + start_byte_offset) / 16u)]);
  float4 v_3 = asfloat(a[((32u + start_byte_offset) / 16u)]);
  return float4x4(v_1, v_2, v_3, asfloat(a[((48u + start_byte_offset) / 16u)]));
}

Inner v_4(uint start_byte_offset) {
  Inner v_5 = {v(start_byte_offset)};
  return v_5;
}

typedef Inner ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      Inner v_9 = v_4((start_byte_offset + (v_8 * 64u)));
      a[v_8] = v_9;
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  Inner v_10[4] = a;
  return v_10;
}

Outer v_11(uint start_byte_offset) {
  Inner v_12[4] = v_6(start_byte_offset);
  Outer v_13 = {v_12};
  return v_13;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_14(uint start_byte_offset) {
  Outer a[4] = (Outer[4])0;
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      Outer v_17 = v_11((start_byte_offset + (v_16 * 256u)));
      a[v_16] = v_17;
      {
        v_15 = (v_16 + 1u);
      }
      continue;
    }
  }
  Outer v_18[4] = a;
  return v_18;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_19 = (256u * uint(i()));
  uint v_20 = (64u * uint(i()));
  uint v_21 = (16u * uint(i()));
  Outer l_a[4] = v_14(0u);
  Outer l_a_i = v_11(v_19);
  Inner l_a_i_a[4] = v_6(v_19);
  Inner l_a_i_a_i = v_4((v_19 + v_20));
  float4x4 l_a_i_a_i_m = v((v_19 + v_20));
  float4 l_a_i_a_i_m_i = asfloat(a[(((v_19 + v_20) + v_21) / 16u)]);
  uint v_22 = (((v_19 + v_20) + v_21) + (uint(i()) * 4u));
  float l_a_i_a_i_m_i_i = asfloat(a[(v_22 / 16u)][((v_22 % 16u) / 4u)]);
}

