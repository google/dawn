
void f() {
  {
    int i = int(0);
    while(true) {
      if (false) {
      } else {
        break;
      }
      {
        i = (i + int(1));
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

