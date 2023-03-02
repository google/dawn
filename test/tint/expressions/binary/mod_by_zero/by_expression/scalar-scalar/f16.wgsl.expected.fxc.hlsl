SKIP: FAILED

float16_t tint_float_mod(float16_t lhs, float16_t rhs) {
  return (lhs - (trunc((lhs / rhs)) * rhs));
}

[numthreads(1, 1, 1)]
void f() {
  float16_t a = float16_t(1.0h);
  float16_t b = float16_t(0.0h);
  const float16_t r = tint_float_mod(a, (b + b));
  return;
}
