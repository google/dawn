[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  const int a = 1;
  const int a__ = a;
  const int b = a;
  const int b__ = a__;
}
