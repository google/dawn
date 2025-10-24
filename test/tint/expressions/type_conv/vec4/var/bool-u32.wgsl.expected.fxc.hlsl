
static bool4 u = (true).xxxx;
[numthreads(1, 1, 1)]
void f() {
  uint4 v = uint4(u);
}

