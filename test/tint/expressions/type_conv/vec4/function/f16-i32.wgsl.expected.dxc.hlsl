
static float16_t t = float16_t(0.0h);
vector<float16_t, 4> m() {
  t = float16_t(1.0h);
  return vector<float16_t, 4>((t).xxxx);
}

int4 tint_v4f16_to_v4i32(vector<float16_t, 4> value) {
  return int4(clamp(value, (float16_t(-65504.0h)).xxxx, (float16_t(65504.0h)).xxxx));
}

void f() {
  int4 v = tint_v4f16_to_v4i32(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

