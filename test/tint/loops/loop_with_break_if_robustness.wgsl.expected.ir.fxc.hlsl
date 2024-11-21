
int f() {
  int i = int(0);
  {
    while(true) {
      if ((i > int(4))) {
        return i;
      }
      {
        i = (i + int(1));
        if ((i == int(4))) { break; }
      }
      continue;
    }
  }
  return i;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

