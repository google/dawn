
static bool3 u = (true).xxx;
[numthreads(1, 1, 1)]
void f() {
  int3 v = int3(u);
}

