
static float16_t t = float16_t(0.0h);
float16_t m() {
  t = float16_t(1.0h);
  return float16_t(t);
}

uint tint_f16_to_u32(float16_t value) {
  return uint(clamp(value, float16_t(0.0h), float16_t(65504.0h)));
}

void f() {
  uint v = tint_f16_to_u32(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

