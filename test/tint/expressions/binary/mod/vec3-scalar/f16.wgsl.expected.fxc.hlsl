SKIP: FAILED

vector<float16_t, 3> tint_float_mod(vector<float16_t, 3> lhs, float16_t rhs) {
  const vector<float16_t, 3> r = vector<float16_t, 3>((rhs).xxx);
  return (lhs - (trunc((lhs / r)) * r));
}

[numthreads(1, 1, 1)]
void f() {
  const vector<float16_t, 3> a = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  const float16_t b = float16_t(4.0h);
  const vector<float16_t, 3> r = tint_float_mod(a, b);
  return;
}
