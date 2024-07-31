
int f() {
  int i = 0;
  {
    while(true) {
      if ((i > 4)) {
        return i;
      }
      {
        i = (i + 1);
        if ((i == 4)) { break; }
      }
      continue;
    }
  }
  return i;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

