
static float2 v2f = (0.0f).xx;
static int3 v3i = (0).xxx;
static uint4 v4u = (0u).xxxx;
static bool2 v2b = (false).xx;
void foo() {
  {
    int i = 0;
    while(true) {
      if ((i < 2)) {
      } else {
        break;
      }
      v2f[i] = 1.0f;
      v3i[i] = 1;
      v4u[i] = 1u;
      v2b[i] = true;
      {
        i = (i + 1);
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void main() {
  {
    int i = 0;
    while(true) {
      if ((i < 2)) {
      } else {
        break;
      }
      foo();
      {
        i = (i + 1);
      }
      continue;
    }
  }
}

