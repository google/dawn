
static float16_t t = float16_t(0.0h);
vector<float16_t, 2> m() {
  t = float16_t(1.0h);
  return vector<float16_t, 2>((t).xx);
}

int2 tint_v2f16_to_v2i32(vector<float16_t, 2> value) {
  return int2(clamp(value, (float16_t(-65504.0h)).xx, (float16_t(65504.0h)).xx));
}

[numthreads(1, 1, 1)]
void f() {
  int2 v = tint_v2f16_to_v2i32(m());
}

