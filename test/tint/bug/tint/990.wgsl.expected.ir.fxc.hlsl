
void f() {
  int i = 0;
  {
    int p = i;
    while(true) {
      if (false) {
      } else {
        break;
      }
      {
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

