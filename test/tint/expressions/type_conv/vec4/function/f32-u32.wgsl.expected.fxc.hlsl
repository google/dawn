
static float t = 0.0f;
float4 m() {
  t = 1.0f;
  return float4((t).xxxx);
}

uint4 tint_v4f32_to_v4u32(float4 value) {
  return uint4(clamp(value, (0.0f).xxxx, (4294967040.0f).xxxx));
}

void f() {
  uint4 v = tint_v4f32_to_v4u32(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

