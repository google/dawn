
cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = asint((asuint(counter) + 1u));
  return counter;
}

float2x4 v(uint start_byte_offset) {
  return float2x4(asfloat(a[(start_byte_offset / 16u)]), asfloat(a[((16u + start_byte_offset) / 16u)]));
}

typedef float2x4 ary_ret[4];
ary_ret v_1(uint start_byte_offset) {
  float2x4 a_1[4] = (float2x4[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_2 = idx;
      if ((v_2 >= 4u)) {
        break;
      }
      a_1[v_2] = v((start_byte_offset + (v_2 * 32u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  float2x4 v_3[4] = a_1;
  return v_3;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_4 = (min(uint(i()), 3u) * 32u);
  uint v_5 = (min(uint(i()), 1u) * 16u);
  float2x4 l_a[4] = v_1(0u);
  float2x4 l_a_i = v(v_4);
  float4 l_a_i_i = asfloat(a[((v_4 + v_5) / 16u)]);
  uint v_6 = (v_4 + v_5);
  s.Store(0u, asuint((((asfloat(a[(v_6 / 16u)][((v_6 & 15u) >> 2u)]) + l_a[int(0)][int(0)].x) + l_a_i[int(0)].x) + l_a_i_i.x)));
}

