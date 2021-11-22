[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int a = 1;
static int a__ = 2;

void f() {
  int b = a;
  int b__ = a__;
}
