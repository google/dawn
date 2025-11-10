
[numthreads(1, 1, 1)]
void f() {
  int i = int(1);
  int b = int2(int(1), int(2))[i];
}

