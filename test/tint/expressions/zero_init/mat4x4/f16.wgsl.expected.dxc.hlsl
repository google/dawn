
void f() {
  matrix<float16_t, 4, 4> v = matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

