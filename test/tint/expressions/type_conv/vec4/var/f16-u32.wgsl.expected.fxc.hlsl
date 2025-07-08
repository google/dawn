SKIP: INVALID


static vector<float16_t, 4> u = (float16_t(1.0h)).xxxx;
uint4 tint_v4f16_to_v4u32(vector<float16_t, 4> value) {
  return uint4(clamp(value, (float16_t(0.0h)).xxxx, (float16_t(65504.0h)).xxxx));
}

void f() {
  uint4 v = tint_v4f16_to_v4u32(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

