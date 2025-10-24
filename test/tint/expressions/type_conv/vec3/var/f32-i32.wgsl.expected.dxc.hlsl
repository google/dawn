
static float3 u = (1.0f).xxx;
int3 tint_v3f32_to_v3i32(float3 value) {
  return int3(clamp(value, (-2147483648.0f).xxx, (2147483520.0f).xxx));
}

[numthreads(1, 1, 1)]
void f() {
  int3 v = tint_v3f32_to_v3i32(u);
}

