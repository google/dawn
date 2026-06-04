
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
static float4x2 p[4] = (float4x2[4])0;
float4x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = u[(v_3 / 16u)];
  float2 v_5 = asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint v_6 = (16u + start_byte_offset);
  uint4 v_7 = u[(v_6 / 16u)];
  float2 v_8 = asfloat((((((v_6 & 15u) >> 2u) == 2u)) ? (v_7.zw) : (v_7.xy)));
  uint v_9 = (24u + start_byte_offset);
  uint4 v_10 = u[(v_9 / 16u)];
  return float4x2(v_2, v_5, v_8, asfloat((((((v_9 & 15u) >> 2u) == 2u)) ? (v_10.zw) : (v_10.xy))));
}

typedef float4x2 ary_ret[4];
ary_ret v_11(uint start_byte_offset) {
  float4x2 a[4] = (float4x2[4])0;
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      a[v_13] = v((start_byte_offset + (v_13 * 32u)));
      {
        v_12 = (v_13 + 1u);
      }
    }
  }
  float4x2 v_14[4] = a;
  return v_14;
}

[numthreads(1, 1, 1)]
void f() {
  float4x2 v_15[4] = v_11(0u);
  p = v_15;
  p[1u] = v(64u);
  p[1u][0u] = asfloat(u[0u].zw).yx;
  p[1u][0u].x = asfloat(u[0u].z);
  s.Store(0u, asuint(p[1u][0u].x));
}

