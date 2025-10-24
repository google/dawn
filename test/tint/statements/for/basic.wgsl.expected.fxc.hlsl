
void some_loop_body() {
}

[numthreads(1, 1, 1)]
void f() {
  {
    int i = int(0);
    while((i < int(5))) {
      some_loop_body();
      {
        i = asint((asuint(i) + asuint(int(1))));
      }
      continue;
    }
  }
}

