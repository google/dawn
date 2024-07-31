
static int t = 0;
int2 m() {
  t = 1;
  return int2((t).xx);
}

void f() {
  float2 v = float2(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

