[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int f() {
  int i = 0;
  int j = 0;
  [loop] while (true) {
    i = (i + 1);
    if ((i > 4)) {
      return 1;
    }
    [loop] while (true) {
      j = (j + 1);
      if ((j > 4)) {
        return 2;
      }
    }
  }
}
