
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
void a(float4x3 m) {
}

void b(float3 v) {
}

void c(float f) {
}

float4x3 v_1(uint start_byte_offset) {
  float3 v_2 = asfloat(u[(start_byte_offset / 16u)].xyz);
  float3 v_3 = asfloat(u[((16u + start_byte_offset) / 16u)].xyz);
  float3 v_4 = asfloat(u[((32u + start_byte_offset) / 16u)].xyz);
  return float4x3(v_2, v_3, v_4, asfloat(u[((48u + start_byte_offset) / 16u)].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  a(v_1(0u));
  b(asfloat(u[1u].xyz));
  b(asfloat(u[1u].xyz).zxy);
  c(asfloat(u[1u].x));
  c(asfloat(u[1u].xyz).zxy[0u]);
}

