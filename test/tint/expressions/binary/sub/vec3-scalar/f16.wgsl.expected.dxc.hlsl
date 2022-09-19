[numthreads(1, 1, 1)]
void f() {
  const vector<float16_t, 3> a = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  const float16_t b = float16_t(4.0h);
  const vector<float16_t, 3> r = (a - b);
  return;
}
