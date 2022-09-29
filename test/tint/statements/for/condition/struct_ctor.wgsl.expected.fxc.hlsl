[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  int i;
};

void f() {
  int i = 0;
  while (true) {
    const S tint_symbol = {1};
    if (!((i < tint_symbol.i))) {
      break;
    }
    {
    }
  }
}
