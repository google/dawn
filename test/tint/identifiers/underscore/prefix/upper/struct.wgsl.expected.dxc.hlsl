[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct _A {
  int _B;
};

void f() {
  const _A c = (_A)0;
  const int d = c._B;
}
