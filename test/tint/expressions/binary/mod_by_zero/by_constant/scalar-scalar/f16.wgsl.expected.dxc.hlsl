[numthreads(1, 1, 1)]
void f() {
  const float16_t r = (float16_t(1.0h) % float16_t(0.0h));
  return;
}
