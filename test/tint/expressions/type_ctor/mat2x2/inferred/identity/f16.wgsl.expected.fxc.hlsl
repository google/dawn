SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static matrix<float16_t, 2, 2> m = matrix<float16_t, 2, 2>(vector<float16_t, 2>(float16_t(0.0h), float16_t(1.0h)), vector<float16_t, 2>(float16_t(2.0h), float16_t(3.0h)));

matrix<float16_t, 2, 2> f() {
  const matrix<float16_t, 2, 2> m_1 = matrix<float16_t, 2, 2>(m);
  return m_1;
}
