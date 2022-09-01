[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float2 u = (1.0f).xx;

void f() {
  const int2 v = int2(u);
}
