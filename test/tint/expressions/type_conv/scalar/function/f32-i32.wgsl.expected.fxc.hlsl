
static float t = 0.0f;
float m() {
  t = 1.0f;
  return float(t);
}

int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}

[numthreads(1, 1, 1)]
void f() {
  int v = tint_f32_to_i32(m());
}

