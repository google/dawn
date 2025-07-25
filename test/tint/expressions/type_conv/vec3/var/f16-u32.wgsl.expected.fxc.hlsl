SKIP: INVALID


static vector<float16_t, 3> u = (float16_t(1.0h)).xxx;
uint3 tint_v3f16_to_v3u32(vector<float16_t, 3> value) {
  return uint3(clamp(value, (float16_t(0.0h)).xxx, (float16_t(65504.0h)).xxx));
}

void f() {
  uint3 v = tint_v3f16_to_v3u32(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

