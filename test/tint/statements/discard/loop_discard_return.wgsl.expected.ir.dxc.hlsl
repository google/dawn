
void f() {
  {
    while(true) {
      discard;
      return;
    }
  }
  /* unreachable */
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

