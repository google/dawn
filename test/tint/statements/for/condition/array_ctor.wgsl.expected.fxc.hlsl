[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  int i = 0;
  while (true) {
    const int tint_symbol[1] = {1};
    if (!((i < tint_symbol[0]))) {
      break;
    }
    {
    }
  }
}
