
static uint3 u = (1u).xxx;
[numthreads(1, 1, 1)]
void f() {
  int3 v = int3(u);
}

