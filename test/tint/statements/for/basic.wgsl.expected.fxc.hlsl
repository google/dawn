
void some_loop_body() {
}

[numthreads(1, 1, 1)]
void f() {
  {
    int i = int(0);
    for( ; (i < int(5)); i = asint((asuint(i) + 1u))) {
      some_loop_body();
    }
  }
}

