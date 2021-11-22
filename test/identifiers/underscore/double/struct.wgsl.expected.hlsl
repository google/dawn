[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct a__ {
  int b__;
};

void f() {
  const a__ c = (a__)0;
  const int d = c.b__;
}
