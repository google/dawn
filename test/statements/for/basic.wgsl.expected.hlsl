[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void some_loop_body() {
}

void f() {
  {
    [loop] for(int i = 0; (i < 5); i = (i + 1)) {
      some_loop_body();
    }
  }
}
