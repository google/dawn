
static vector<float16_t, 2> u = (float16_t(1.0h)).xx;
int2 tint_v2f16_to_v2i32(vector<float16_t, 2> value) {
  return int2(clamp(value, (float16_t(-65504.0h)).xx, (float16_t(65504.0h)).xx));
}

void f() {
  int2 v = tint_v2f16_to_v2i32(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

