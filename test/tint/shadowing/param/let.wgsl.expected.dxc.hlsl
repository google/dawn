[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f(int a) {
  {
    const int a_1 = a;
    const int b = a_1;
  }
}
