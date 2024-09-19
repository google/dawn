
cbuffer cbuffer_u : register(b0) {
  uint4 u[12];
};
RWByteAddressBuffer s : register(u1);
static float3x4 p[4] = (float3x4[4])0;
float3x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(u[(start_byte_offset / 16u)]);
  float4 v_2 = asfloat(u[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_1, v_2, asfloat(u[((32u + start_byte_offset) / 16u)]));
}

typedef float3x4 ary_ret[4];
ary_ret v_3(uint start_byte_offset) {
  float3x4 a[4] = (float3x4[4])0;
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      a[v_5] = v((start_byte_offset + (v_5 * 48u)));
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  float3x4 v_6[4] = a;
  return v_6;
}

[numthreads(1, 1, 1)]
void f() {
  float3x4 v_7[4] = v_3(0u);
  p = v_7;
  p[int(1)] = v(96u);
  p[int(1)][int(0)] = asfloat(u[1u]).ywxz;
  p[int(1)][int(0)][0u] = asfloat(u[1u].x);
  s.Store(0u, asuint(p[int(1)][int(0)].x));
}

