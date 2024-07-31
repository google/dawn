
int f() {
  int i = 0;
  {
    while(true) {
      if ((i < 4)) {
      } else {
        break;
      }
      i = (i + 1);
      {
      }
      continue;
    }
  }
  return i;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

