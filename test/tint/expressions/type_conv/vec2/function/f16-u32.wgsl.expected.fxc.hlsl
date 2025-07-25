SKIP: INVALID


static float16_t t = float16_t(0.0h);
vector<float16_t, 2> m() {
  t = float16_t(1.0h);
  return vector<float16_t, 2>((t).xx);
}

uint2 tint_v2f16_to_v2u32(vector<float16_t, 2> value) {
  return uint2(clamp(value, (float16_t(0.0h)).xx, (float16_t(65504.0h)).xx));
}

void f() {
  uint2 v = tint_v2f16_to_v2u32(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

