[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  const int i = 1;
  int b = int2(1, 2)[i];
}
