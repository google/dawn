SKIP: INVALID


static float16_t u = float16_t(1.0h);
int tint_f16_to_i32(float16_t value) {
  return int(clamp(value, float16_t(-65504.0h), float16_t(65504.0h)));
}

void f() {
  int v = tint_f16_to_i32(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

