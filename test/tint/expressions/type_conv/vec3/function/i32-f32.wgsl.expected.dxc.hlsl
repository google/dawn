
static int t = int(0);
int3 m() {
  t = int(1);
  return int3((t).xxx);
}

[numthreads(1, 1, 1)]
void f() {
  float3 v = float3(m());
}

