
int f() {
  int i = int(0);
  {
    while(true) {
      i = (i + int(1));
      if ((i > int(4))) {
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

