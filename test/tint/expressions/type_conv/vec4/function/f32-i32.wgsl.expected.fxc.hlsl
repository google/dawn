
static float t = 0.0f;
float4 m() {
  t = 1.0f;
  return float4((t).xxxx);
}

int4 tint_v4f32_to_v4i32(float4 value) {
  return int4(clamp(value, (-2147483648.0f).xxxx, (2147483520.0f).xxxx));
}

[numthreads(1, 1, 1)]
void f() {
  int4 v = tint_v4f32_to_v4i32(m());
}

