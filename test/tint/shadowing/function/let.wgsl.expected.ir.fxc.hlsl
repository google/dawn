SKIP: FAILED

void a() {
  int a = 1;
  int b = a;
  int a = 1;
  int b = a;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

