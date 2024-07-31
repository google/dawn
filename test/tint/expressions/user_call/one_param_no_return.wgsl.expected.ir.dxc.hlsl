
void c(int z) {
  int a = (1 + z);
  a = (a + 2);
}

void b() {
  c(2);
  c(3);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

