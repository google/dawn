[numthreads(1, 1, 1)]
void f() {
  const float16_t a = float16_t(1.0h);
  const float16_t b = a;
  return;
}
