[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  int i;
};

void f() {
  const S tint_symbol = {1};
  {
    for(int i = tint_symbol.i; false; ) {
    }
  }
}
