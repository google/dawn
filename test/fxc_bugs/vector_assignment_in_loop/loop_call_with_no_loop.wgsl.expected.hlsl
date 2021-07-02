static float2 v2f = float2(0.0f, 0.0f);
static int3 v3i = int3(0, 0, 0);
static uint4 v4u = uint4(0u, 0u, 0u, 0u);
static bool2 v2b = bool2(false, false);

void foo() {
  int i = 0;
  v2f[i] = 1.0f;
  v3i[i] = 1;
  v4u[i] = 1u;
  v2b[i] = true;
}

[numthreads(1, 1, 1)]
void main() {
  {
    int i = 0;
    for(; !(!((i < 2))); i = (i + 1)) {
      foo();
    }
  }
  return;
}
