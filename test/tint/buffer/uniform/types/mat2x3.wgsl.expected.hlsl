cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[2];
};

float2x3 tint_symbol(uint4 buffer[2], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz));
}

[numthreads(1, 1, 1)]
void main() {
  const float2x3 x = tint_symbol(u, 0u);
  return;
}
