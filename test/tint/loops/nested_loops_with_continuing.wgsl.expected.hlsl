[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int f() {
  int i = 0;
  int j = 0;
  [loop] while (true) {
    if ((i > 4)) {
      return 1;
    }
    [loop] while (true) {
      if ((j > 4)) {
        return 2;
      }
      {
        j = (j + 1);
      }
    }
    {
      i = (i + 1);
    }
  }
}
