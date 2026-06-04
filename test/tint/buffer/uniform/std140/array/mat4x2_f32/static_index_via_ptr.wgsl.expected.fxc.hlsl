
cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
};
RWByteAddressBuffer s : register(u1);
float4x2 v(uint start_byte_offset) {
  uint4 v_1 = a[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = a[(v_3 / 16u)];
  float2 v_5 = asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint v_6 = (16u + start_byte_offset);
  uint4 v_7 = a[(v_6 / 16u)];
  float2 v_8 = asfloat((((((v_6 & 15u) >> 2u) == 2u)) ? (v_7.zw) : (v_7.xy)));
  uint v_9 = (24u + start_byte_offset);
  uint4 v_10 = a[(v_9 / 16u)];
  return float4x2(v_2, v_5, v_8, asfloat((((((v_9 & 15u) >> 2u) == 2u)) ? (v_10.zw) : (v_10.xy))));
}

typedef float4x2 ary_ret[4];
ary_ret v_11(uint start_byte_offset) {
  float4x2 a_1[4] = (float4x2[4])0;
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      a_1[v_13] = v((start_byte_offset + (v_13 * 32u)));
      {
        v_12 = (v_13 + 1u);
      }
    }
  }
  float4x2 v_14[4] = a_1;
  return v_14;
}

[numthreads(1, 1, 1)]
void f() {
  float4x2 l_a[4] = v_11(0u);
  float4x2 l_a_i = v(64u);
  float2 l_a_i_i = asfloat(a[4u].zw);
  s.Store(0u, asuint((((asfloat(a[4u].z) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x)));
}

