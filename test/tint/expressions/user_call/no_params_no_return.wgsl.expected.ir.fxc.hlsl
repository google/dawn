SKIP: FAILED

void c() {
  int a = 1;
  a = (a + 2);
}

void b() {
  c();
  c();
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

