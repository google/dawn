cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};

float2x4 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(u[scalar_offset / 4]), asfloat(u[scalar_offset_1 / 4]));
}

[numthreads(1, 1, 1)]
void main() {
  const float2x4 x = u_load(0u);
  return;
}
