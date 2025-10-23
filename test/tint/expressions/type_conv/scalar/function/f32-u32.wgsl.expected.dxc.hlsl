
static float t = 0.0f;
float m() {
  t = 1.0f;
  return float(t);
}

uint tint_f32_to_u32(float value) {
  return uint(clamp(value, 0.0f, 4294967040.0f));
}

[numthreads(1, 1, 1)]
void f() {
  uint v = tint_f32_to_u32(m());
}

