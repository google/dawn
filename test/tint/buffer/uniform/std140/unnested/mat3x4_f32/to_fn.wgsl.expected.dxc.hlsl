cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[3];
};

void a(float3x4 m) {
}

void b(float4 v) {
}

void c(float f_1) {
}

float3x4 tint_symbol(uint4 buffer[3], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]));
}

[numthreads(1, 1, 1)]
void f() {
  a(tint_symbol(u, 0u));
  b(asfloat(u[1]));
  b(asfloat(u[1]).ywxz);
  c(asfloat(u[1].x));
  c(asfloat(u[1]).ywxz.x);
  return;
}
