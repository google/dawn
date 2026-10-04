
cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = asint((asuint(counter) + 1u));
  return counter;
}

float4x2 v(uint start_byte_offset) {
  uint4 v_1 = a[(start_byte_offset / 16u)];
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = a[(v_2 / 16u)];
  uint v_4 = (16u + start_byte_offset);
  uint4 v_5 = a[(v_4 / 16u)];
  uint v_6 = (24u + start_byte_offset);
  uint4 v_7 = a[(v_6 / 16u)];
  return float4x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)), asfloat(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)), asfloat(select((((v_6 & 15u) >> 2u) == 2u), v_7.zw, v_7.xy)));
}

typedef float4x2 ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  float4x2 a_1[4] = (float4x2[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_9 = idx;
      if ((v_9 >= 4u)) {
        break;
      }
      a_1[v_9] = v((start_byte_offset + (v_9 * 32u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  float4x2 v_10[4] = a_1;
  return v_10;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_11 = (min(uint(i()), 3u) * 32u);
  uint v_12 = (min(uint(i()), 3u) * 8u);
  float4x2 l_a[4] = v_8(0u);
  float4x2 l_a_i = v(v_11);
  uint v_13 = (v_11 + v_12);
  uint4 v_14 = a[(v_13 / 16u)];
  float2 l_a_i_i = asfloat(select((((v_13 & 15u) >> 2u) == 2u), v_14.zw, v_14.xy));
  uint v_15 = (v_11 + v_12);
  s.Store(0u, asuint((((asfloat(a[(v_15 / 16u)][((v_15 & 15u) >> 2u)]) + l_a[int(0)][int(0)].x) + l_a_i[int(0)].x) + l_a_i_i.x)));
}

