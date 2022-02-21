[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int a = 1;
static int _a = 2;

void f() {
  int b = a;
  int _b = _a;
}
