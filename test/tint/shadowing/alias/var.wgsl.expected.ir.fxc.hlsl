SKIP: FAILED

void f() {
  int a = 0;
  int b = a;
  int a = 0;
  int b = a;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

