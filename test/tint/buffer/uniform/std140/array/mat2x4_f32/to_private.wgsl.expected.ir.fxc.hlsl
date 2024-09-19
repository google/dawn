
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
static float2x4 p[4] = (float2x4[4])0;
float2x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(u[(start_byte_offset / 16u)]);
  return float2x4(v_1, asfloat(u[((16u + start_byte_offset) / 16u)]));
}

typedef float2x4 ary_ret[4];
ary_ret v_2(uint start_byte_offset) {
  float2x4 a[4] = (float2x4[4])0;
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      a[v_4] = v((start_byte_offset + (v_4 * 32u)));
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
  float2x4 v_5[4] = a;
  return v_5;
}

[numthreads(1, 1, 1)]
void f() {
  float2x4 v_6[4] = v_2(0u);
  p = v_6;
  p[int(1)] = v(64u);
  p[int(1)][int(0)] = asfloat(u[1u]).ywxz;
  p[int(1)][int(0)][0u] = asfloat(u[1u].x);
  s.Store(0u, asuint(p[int(1)][int(0)].x));
}

