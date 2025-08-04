
void some_loop_body() {
}

void f() {
  {
    int i = int(0);
    while((i < int(5))) {
      some_loop_body();
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

