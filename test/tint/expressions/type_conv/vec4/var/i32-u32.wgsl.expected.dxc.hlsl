[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int4 u = (1).xxxx;

void f() {
  const uint4 v = uint4(u);
}
