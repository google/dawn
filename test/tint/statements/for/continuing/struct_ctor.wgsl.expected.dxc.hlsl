[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  int i;
};

void f() {
  {
    int i = 0;
    [loop] while (true) {
      if (true) {
        break;
      }
      {
      }
      {
        const S tint_symbol = {1};
        i = (i + tint_symbol.i);
      }
    }
  }
}
