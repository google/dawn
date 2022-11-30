cbuffer cbuffer_m : register(b0, space0) {
  uint4 m[1];
};
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

float2x2 tint_symbol(uint4 buffer[1], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

[numthreads(1, 1, 1)]
void f() {
  const float2x2 l_m = tint_symbol(m, 0u);
  const float2 l_m_1 = asfloat(m[0].zw);
  return;
}
