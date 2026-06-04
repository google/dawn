
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
static float2x2 p[4] = (float2x2[4])0;
float2x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = u[(v_3 / 16u)];
  return float2x2(v_2, asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy))));
}

typedef float2x2 ary_ret[4];
ary_ret v_5(uint start_byte_offset) {
  float2x2 a[4] = (float2x2[4])0;
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      a[v_7] = v((start_byte_offset + (v_7 * 16u)));
      {
        v_6 = (v_7 + 1u);
      }
    }
  }
  float2x2 v_8[4] = a;
  return v_8;
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 v_9[4] = v_5(0u);
  p = v_9;
  p[1u] = v(32u);
  p[1u][0u] = asfloat(u[0u].zw).yx;
  p[1u][0u].x = asfloat(u[0u].z);
  s.Store(0u, asuint(p[1u][0u].x));
}

