[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  const int a = 1;
  const int _a = a;
  const int b = a;
  const int _b = _a;
}
