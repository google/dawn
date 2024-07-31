
int f() {
  int i = 0;
  {
    while(true) {
      i = (i + 1);
      if ((i > 4)) {
        return i;
      }
      {
      }
      continue;
    }
  }
  /* unreachable */
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

