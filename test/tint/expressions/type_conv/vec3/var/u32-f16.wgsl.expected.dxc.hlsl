[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint3 u = (1u).xxx;

void f() {
  const vector<float16_t, 3> v = vector<float16_t, 3>(u);
}
