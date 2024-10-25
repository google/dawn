
cbuffer cbuffer_a : register(b0) {
  uint4 a[16];
};
RWByteAddressBuffer s : register(u1);
float4x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(a[(start_byte_offset / 16u)]);
  float4 v_2 = asfloat(a[((16u + start_byte_offset) / 16u)]);
  float4 v_3 = asfloat(a[((32u + start_byte_offset) / 16u)]);
  return float4x4(v_1, v_2, v_3, asfloat(a[((48u + start_byte_offset) / 16u)]));
}

typedef float4x4 ary_ret[4];
ary_ret v_4(uint start_byte_offset) {
  float4x4 a_1[4] = (float4x4[4])0;
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      a_1[v_6] = v((start_byte_offset + (v_6 * 64u)));
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  float4x4 v_7[4] = a_1;
  return v_7;
}

[numthreads(1, 1, 1)]
void f() {
  float4x4 l_a[4] = v_4(0u);
  float4x4 l_a_i = v(128u);
  float4 l_a_i_i = asfloat(a[9u]);
  s.Store(0u, asuint((((asfloat(a[9u].x) + l_a[int(0)][int(0)][0u]) + l_a_i[int(0)][0u]) + l_a_i_i[0u])));
}

