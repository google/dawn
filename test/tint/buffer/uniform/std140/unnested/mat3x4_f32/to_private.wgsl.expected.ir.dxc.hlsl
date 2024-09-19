
cbuffer cbuffer_u : register(b0) {
  uint4 u[3];
};
static float3x4 p = float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
float3x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(u[(start_byte_offset / 16u)]);
  float4 v_2 = asfloat(u[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_1, v_2, asfloat(u[((32u + start_byte_offset) / 16u)]));
}

[numthreads(1, 1, 1)]
void f() {
  p = v(0u);
  p[int(1)] = asfloat(u[0u]);
  p[int(1)] = asfloat(u[0u]).ywxz;
  p[int(0)][int(1)] = asfloat(u[1u].x);
}

