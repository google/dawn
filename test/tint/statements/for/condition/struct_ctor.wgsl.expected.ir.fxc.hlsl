
void f() {
  int i = int(0);
  {
    while(true) {
      if ((i < int(1))) {
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

