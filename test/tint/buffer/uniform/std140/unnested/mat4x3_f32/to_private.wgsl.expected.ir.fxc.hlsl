
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
static float4x3 p = float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
float4x3 v(uint start_byte_offset) {
  float3 v_1 = asfloat(u[(start_byte_offset / 16u)].xyz);
  float3 v_2 = asfloat(u[((16u + start_byte_offset) / 16u)].xyz);
  float3 v_3 = asfloat(u[((32u + start_byte_offset) / 16u)].xyz);
  return float4x3(v_1, v_2, v_3, asfloat(u[((48u + start_byte_offset) / 16u)].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  p = v(0u);
  p[int(1)] = asfloat(u[0u].xyz);
  p[int(1)] = asfloat(u[0u].xyz).zxy;
  p[int(0)][int(1)] = asfloat(u[1u].x);
}

