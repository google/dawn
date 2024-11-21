
int f() {
  int i = int(0);
  int j = int(0);
  {
    while(true) {
      i = (i + int(1));
      if ((i > int(4))) {
        return int(1);
      }
      {
        while(true) {
          j = (j + int(1));
          if ((j > int(4))) {
            return int(2);
          }
          {
          }
          continue;
        }
      }
      /* unreachable */
    }
  }
  /* unreachable */
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

