
static float16_t u = float16_t(1.0h);
uint tint_f16_to_u32(float16_t value) {
  return uint(clamp(value, float16_t(0.0h), float16_t(65504.0h)));
}

[numthreads(1, 1, 1)]
void f() {
  uint v = tint_f16_to_u32(u);
}

