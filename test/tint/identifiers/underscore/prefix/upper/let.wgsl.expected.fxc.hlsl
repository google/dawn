[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  const int A = 1;
  const int _A = 2;
  const int B = A;
  const int _B = _A;
}
