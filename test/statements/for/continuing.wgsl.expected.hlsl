[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  int i = 0;
  while (true) {
    {
      i = (i + 1);
    }
  }
}
