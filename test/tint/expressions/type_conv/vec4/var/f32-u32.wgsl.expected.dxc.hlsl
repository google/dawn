
static float4 u = (1.0f).xxxx;
uint4 tint_v4f32_to_v4u32(float4 value) {
  return uint4(clamp(value, (0.0f).xxxx, (4294967040.0f).xxxx));
}

[numthreads(1, 1, 1)]
void f() {
  uint4 v = tint_v4f32_to_v4u32(u);
}

