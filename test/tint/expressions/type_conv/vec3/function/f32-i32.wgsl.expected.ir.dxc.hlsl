
static float t = 0.0f;
float3 m() {
  t = 1.0f;
  return float3((t).xxx);
}

int3 tint_v3f32_to_v3i32(float3 value) {
  return int3(clamp(value, (-2147483648.0f).xxx, (2147483520.0f).xxx));
}

void f() {
  int3 v = tint_v3f32_to_v3i32(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

