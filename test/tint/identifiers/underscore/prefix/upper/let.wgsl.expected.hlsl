[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static const int A = 1;
static const int _A = 2;

void f() {
  const int B = A;
  const int _B = _A;
}
