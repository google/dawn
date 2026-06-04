
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
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = a[(v_2 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)));
}

typedef float2x2 ary_ret[4];
ary_ret v_4(uint start_byte_offset) {
  float2x2 a_1[4] = (float2x2[4])0;
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      a_1[v_6] = v((start_byte_offset + (v_6 * 16u)));
      {
        v_5 = (v_6 + 1u);
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
  float2x2 l_a[4] = v_4(0u);
  float2x2 l_a_i = v(v_8);
  uint v_10 = (v_8 + v_9);
  uint4 v_11 = a[(v_10 / 16u)];
  float2 l_a_i_i = asfloat(select((((v_10 & 15u) >> 2u) == 2u), v_11.zw, v_11.xy));
  uint v_12 = (v_8 + v_9);
  s.Store(0u, asuint((((asfloat(a[(v_12 / 16u)][((v_12 & 15u) >> 2u)]) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x)));
}

