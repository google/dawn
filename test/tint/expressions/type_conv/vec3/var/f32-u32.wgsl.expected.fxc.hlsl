
static float3 u = (1.0f).xxx;
uint3 tint_v3f32_to_v3u32(float3 value) {
  return uint3(clamp(value, (0.0f).xxx, (4294967040.0f).xxx));
}

[numthreads(1, 1, 1)]
void f() {
  uint3 v = tint_v3f32_to_v3u32(u);
}

