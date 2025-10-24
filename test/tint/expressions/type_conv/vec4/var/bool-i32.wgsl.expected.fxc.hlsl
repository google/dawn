
static bool4 u = (true).xxxx;
[numthreads(1, 1, 1)]
void f() {
  int4 v = int4(u);
}

