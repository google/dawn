
void f() {
  {
    int i = 0;
    while(true) {
      if (false) {
      } else {
        break;
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

