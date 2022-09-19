[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void a() {
  {
    int a_1 = 1;
    int b = a_1;
  }
  const int a_2 = 1;
  const int b = a_2;
}
