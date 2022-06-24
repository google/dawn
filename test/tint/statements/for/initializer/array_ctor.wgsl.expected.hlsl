[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  const int tint_symbol[1] = {1};
  {
    [loop] for(int i = 1; false; ) {
    }
  }
}
