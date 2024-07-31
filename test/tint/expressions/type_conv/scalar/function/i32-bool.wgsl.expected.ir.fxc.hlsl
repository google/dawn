
static int t = 0;
int m() {
  t = 1;
  return int(t);
}

void f() {
  bool v = bool(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

