
static bool continue_execution = true;
void f() {
  {
    while(true) {
      continue_execution = false;
      return;
    }
  }
  /* unreachable */
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

