
void some_loop_body() {
}

[numthreads(1, 1, 1)]
void f() {
  {
    int i = int(0);
    while(true) {
      if ((i < int(5))) {
      } else {
        break;
      }
      some_loop_body();
      {
        i = asint((asuint(i) + asuint(int(1))));
      }
      continue;
    }
  }
}

