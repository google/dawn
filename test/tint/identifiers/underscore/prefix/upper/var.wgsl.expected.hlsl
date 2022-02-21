[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int A = 1;
static int _A = 2;

void f() {
  int B = A;
  int _B = _A;
}
