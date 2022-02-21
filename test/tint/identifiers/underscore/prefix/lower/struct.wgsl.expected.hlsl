[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct _a {
  int _b;
};

void f() {
  const _a c = (_a)0;
  const int d = c._b;
}
