
static bool2 u = (true).xx;
[numthreads(1, 1, 1)]
void f() {
  int2 v = int2(u);
}

