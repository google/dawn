[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  vector<float16_t, 2> v2 = vector<float16_t, 2>(((float16_t(1.0h) + float16_t(2.0h))).xx);
  vector<float16_t, 3> v3 = vector<float16_t, 3>(((float16_t(1.0h) + float16_t(2.0h))).xxx);
  vector<float16_t, 4> v4 = vector<float16_t, 4>(((float16_t(1.0h) + float16_t(2.0h))).xxxx);
}
