struct Inner {
  float3x3 m;
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

float3x3 v(uint start_byte_offset) {
  float3 v_1 = asfloat(a[(start_byte_offset / 16u)].xyz);
  float3 v_2 = asfloat(a[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_1, v_2, asfloat(a[((32u + start_byte_offset) / 16u)].xyz));
}

Inner v_3(uint start_byte_offset) {
  Inner v_4 = {v(start_byte_offset)};
  return v_4;
}

typedef Inner ary_ret[4];
ary_ret v_5(uint start_byte_offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      Inner v_8 = v_3((start_byte_offset + (v_7 * 64u)));
      a[v_7] = v_8;
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  Inner v_9[4] = a;
  return v_9;
}

Outer v_10(uint start_byte_offset) {
  Inner v_11[4] = v_5(start_byte_offset);
  Outer v_12 = {v_11};
  return v_12;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_13(uint start_byte_offset) {
  Outer a[4] = (Outer[4])0;
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      Outer v_16 = v_10((start_byte_offset + (v_15 * 256u)));
      a[v_15] = v_16;
      {
        v_14 = (v_15 + 1u);
      }
      continue;
    }
  }
  Outer v_17[4] = a;
  return v_17;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_18 = (256u * uint(i()));
  uint v_19 = (64u * uint(i()));
  uint v_20 = (16u * uint(i()));
  Outer l_a[4] = v_13(0u);
  Outer l_a_i = v_10(v_18);
  Inner l_a_i_a[4] = v_5(v_18);
  Inner l_a_i_a_i = v_3((v_18 + v_19));
  float3x3 l_a_i_a_i_m = v((v_18 + v_19));
  float3 l_a_i_a_i_m_i = asfloat(a[(((v_18 + v_19) + v_20) / 16u)].xyz);
  uint v_21 = (((v_18 + v_19) + v_20) + (uint(i()) * 4u));
  float l_a_i_a_i_m_i_i = asfloat(a[(v_21 / 16u)][((v_21 % 16u) / 4u)]);
}

