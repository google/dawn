
static const uint v_1[2] = {1u, 2u};
static uint arr[2] = v_1;
[numthreads(1, 1, 1)]
void f() {
  uint v[2] = arr;
}

