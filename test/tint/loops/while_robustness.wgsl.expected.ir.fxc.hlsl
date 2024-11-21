
int f() {
  int i = int(0);
  {
    while(true) {
      if ((i < int(4))) {
      } else {
        break;
      }
      i = (i + int(1));
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

