
static uint3 u = (1u).xxx;
[numthreads(1, 1, 1)]
void f() {
  bool3 v = bool3(u);
}

