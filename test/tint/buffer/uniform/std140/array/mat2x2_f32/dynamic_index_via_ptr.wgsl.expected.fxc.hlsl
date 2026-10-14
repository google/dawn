
cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = asint((asuint(counter) + 1u));
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
    uint idx = 0u;
    while(true) {
      uint v_6 = idx;
      if ((v_6 >= 4u)) {
        break;
      }
      a_1[v_6] = v((start_byte_offset + (v_6 * 16u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  float2x2 v_7[4] = a_1;
  return v_7;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_8 = (min(uint(i()), 3u) * 16u);
  uint v_9 = (min(uint(i()), 1u) * 8u);
  float2x2 l_a[4] = v_5(0u);
  float2x2 l_a_i = v(v_8);
  uint v_10 = (v_8 + v_9);
  uint4 v_11 = a[(v_10 / 16u)];
  float2 l_a_i_i = asfloat((((((v_10 & 15u) >> 2u) == 2u)) ? (v_11.zw) : (v_11.xy)));
  uint v_12 = (v_8 + v_9);
  s.Store(0u, asuint((((asfloat(a[(v_12 / 16u)][((v_12 & 15u) >> 2u)]) + l_a[int(0)][int(0)].x) + l_a_i[int(0)].x) + l_a_i_i.x)));
}

