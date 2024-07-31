
int c(int x, int y, int z) {
  int a = (((1 + x) + y) + z);
  a = (a + 2);
  return a;
}

void b() {
  int b_1 = c(2, 3, 4);
  int v = c(3, 4, 5);
  b_1 = (b_1 + v);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

