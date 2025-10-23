
static float u = 1.0f;
uint tint_f32_to_u32(float value) {
  return uint(clamp(value, 0.0f, 4294967040.0f));
}

[numthreads(1, 1, 1)]
void f() {
  uint v = tint_f32_to_u32(u);
}

