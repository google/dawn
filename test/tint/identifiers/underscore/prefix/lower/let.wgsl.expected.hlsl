[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static const int a = 1;
static const int _a = 2;

void f() {
  const int b = 1;
  const int _b = 2;
}
