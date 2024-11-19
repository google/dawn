
cbuffer cbuffer_u : register(b0) {
  uint4 u[3];
};
static float3x3 p = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
float3x3 v(uint start_byte_offset) {
  return float3x3(asfloat(u[(start_byte_offset / 16u)].xyz), asfloat(u[((16u + start_byte_offset) / 16u)].xyz), asfloat(u[((32u + start_byte_offset) / 16u)].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  p = v(0u);
  p[int(1)] = asfloat(u[0u].xyz);
  p[int(1)] = asfloat(u[0u].xyz).zxy;
  p[int(0)].y = asfloat(u[1u].x);
}

