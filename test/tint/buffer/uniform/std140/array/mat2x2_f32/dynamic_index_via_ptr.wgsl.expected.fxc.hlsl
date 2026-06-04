
cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = asint((asuint(counter) + asuint(int(1))));
  return counter;
}

float2x2 v(uint start_byte_offset) {
  uint4 v_1 = a[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = a[(v_3 / 16u)];
  return float2x2(v_2, asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy))));
}

typedef float2x2 ary_ret[4];
ary_ret v_5(uint start_byte_offset) {
  float2x2 a_1[4] = (float2x2[4])0;
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      a_1[v_7] = v((start_byte_offset + (v_7 * 16u)));
      {
        v_6 = (v_7 + 1u);
      }
    }
  }
  float2x2 v_8[4] = a_1;
  return v_8;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_9 = (min(uint(i()), 3u) * 16u);
  uint v_10 = (min(uint(i()), 1u) * 8u);
  float2x2 l_a[4] = v_5(0u);
  float2x2 l_a_i = v(v_9);
  uint v_11 = (v_9 + v_10);
  uint4 v_12 = a[(v_11 / 16u)];
  float2 l_a_i_i = asfloat((((((v_11 & 15u) >> 2u) == 2u)) ? (v_12.zw) : (v_12.xy)));
  uint v_13 = (v_9 + v_10);
  s.Store(0u, asuint((((asfloat(a[(v_13 / 16u)][((v_13 & 15u) >> 2u)]) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x)));
}

