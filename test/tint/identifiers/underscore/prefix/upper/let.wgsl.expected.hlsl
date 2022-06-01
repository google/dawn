[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static const int A = 1;
static const int _A = 2;

void f() {
  const int B = 1;
  const int _B = 2;
}
