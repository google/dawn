int c() {
  int a = 1;
  a = (a + 2);
  return a;
}

void b() {
  int b = c();
  b = (b + c());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

