
cbuffer cbuffer_a : register(b0) {
  uint4 a[16];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
}

float4x3 v(uint start_byte_offset) {
  float3 v_1 = asfloat(a[(start_byte_offset / 16u)].xyz);
  float3 v_2 = asfloat(a[((16u + start_byte_offset) / 16u)].xyz);
  float3 v_3 = asfloat(a[((32u + start_byte_offset) / 16u)].xyz);
  return float4x3(v_1, v_2, v_3, asfloat(a[((48u + start_byte_offset) / 16u)].xyz));
}

typedef float4x3 ary_ret[4];
ary_ret v_4(uint start_byte_offset) {
  float4x3 a[4] = (float4x3[4])0;
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      a[v_6] = v((start_byte_offset + (v_6 * 64u)));
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  float4x3 v_7[4] = a;
  return v_7;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_8 = (64u * uint(i()));
  uint v_9 = (16u * uint(i()));
  float4x3 v_10[4] = v_4(0u);
  float4x3 l_a_i = v(v_8);
  float3 l_a_i_i = asfloat(a[((v_8 + v_9) / 16u)].xyz);
  float4x3 l_a[4] = v_10;
  s.Store(0u, asuint((((asfloat(a[((v_8 + v_9) / 16u)][(((v_8 + v_9) % 16u) / 4u)]) + l_a[int(0)][int(0)][0u]) + l_a_i[int(0)][0u]) + l_a_i_i[0u])));
}

