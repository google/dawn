
cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
}

float2x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(a[(start_byte_offset / 16u)]);
  return float2x4(v_1, asfloat(a[((16u + start_byte_offset) / 16u)]));
}

typedef float2x4 ary_ret[4];
ary_ret v_2(uint start_byte_offset) {
  float2x4 a_1[4] = (float2x4[4])0;
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      a_1[v_4] = v((start_byte_offset + (v_4 * 32u)));
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
  float2x4 v_5[4] = a_1;
  return v_5;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_6 = (32u * uint(i()));
  uint v_7 = (16u * uint(i()));
  float2x4 l_a[4] = v_2(0u);
  float2x4 l_a_i = v(v_6);
  float4 l_a_i_i = asfloat(a[((v_6 + v_7) / 16u)]);
  s.Store(0u, asuint((((asfloat(a[((v_6 + v_7) / 16u)][(((v_6 + v_7) % 16u) / 4u)]) + l_a[int(0)][int(0)][0u]) + l_a_i[int(0)][0u]) + l_a_i_i[0u])));
}

