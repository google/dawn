
void a() {
  int a_1 = int(0);
  switch(a_1) {
    default:
    case int(0):
    {
      return;
    }
  }
  /* unreachable */
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

