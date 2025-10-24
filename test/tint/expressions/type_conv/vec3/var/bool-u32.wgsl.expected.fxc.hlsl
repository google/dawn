
static bool3 u = (true).xxx;
[numthreads(1, 1, 1)]
void f() {
  uint3 v = uint3(u);
}

