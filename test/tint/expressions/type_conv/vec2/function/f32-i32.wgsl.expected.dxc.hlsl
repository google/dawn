
static float t = 0.0f;
float2 m() {
  t = 1.0f;
  return float2((t).xx);
}

int2 tint_v2f32_to_v2i32(float2 value) {
  return int2(clamp(value, (-2147483648.0f).xx, (2147483520.0f).xx));
}

[numthreads(1, 1, 1)]
void f() {
  int2 v = tint_v2f32_to_v2i32(m());
}

