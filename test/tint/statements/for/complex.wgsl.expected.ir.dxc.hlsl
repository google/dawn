
void some_loop_body() {
}

void f() {
  int j = int(0);
  {
    int i = int(0);
    while(true) {
      bool v = false;
      if ((i < int(5))) {
        v = (j < int(10));
      } else {
        v = false;
      }
      if (v) {
      } else {
        break;
      }
      some_loop_body();
      j = (i * int(30));
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

