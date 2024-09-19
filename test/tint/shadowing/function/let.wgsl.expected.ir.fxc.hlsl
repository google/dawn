
void a() {
  int a_2 = int(1);
  int b = a_2;
  int a_1 = int(1);
  int b_1 = a_1;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

