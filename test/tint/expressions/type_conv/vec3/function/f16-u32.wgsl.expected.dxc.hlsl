
static float16_t t = float16_t(0.0h);
vector<float16_t, 3> m() {
  t = float16_t(1.0h);
  return vector<float16_t, 3>((t).xxx);
}

uint3 tint_v3f16_to_v3u32(vector<float16_t, 3> value) {
  return uint3(clamp(value, (float16_t(0.0h)).xxx, (float16_t(65504.0h)).xxx));
}

void f() {
  uint3 v = tint_v3f16_to_v3u32(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

