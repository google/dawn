cbuffer cbuffer_m : register(b0, space0) {
  uint4 m[3];
};
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

float3x4 tint_symbol(uint4 buffer[3], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]));
}

[numthreads(1, 1, 1)]
void f() {
  const float3x4 l_m = tint_symbol(m, 0u);
  const float4 l_m_1 = asfloat(m[1]);
  return;
}
