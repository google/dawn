[numthreads(1, 1, 1)]
void f() {
  const float16_t a = float16_t(4.0h);
  const vector<float16_t, 3> b = vector<float16_t, 3>(float16_t(0.0h), float16_t(2.0h), float16_t(0.0h));
  const vector<float16_t, 3> r = (a / b);
  return;
}
