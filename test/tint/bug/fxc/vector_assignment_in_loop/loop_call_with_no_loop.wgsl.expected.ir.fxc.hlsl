
static float2 v2f = (0.0f).xx;
static int3 v3i = (int(0)).xxx;
static uint4 v4u = (0u).xxxx;
static bool2 v2b = (false).xx;
void foo() {
  int i = int(0);
  v2f[i] = 1.0f;
  v3i[i] = int(1);
  v4u[i] = 1u;
  v2b[i] = true;
}

[numthreads(1, 1, 1)]
void main() {
  {
    int i = int(0);
    while(true) {
      if ((i < int(2))) {
      } else {
        break;
      }
      foo();
      {
        i = (i + int(1));
      }
      continue;
    }
  }
}

