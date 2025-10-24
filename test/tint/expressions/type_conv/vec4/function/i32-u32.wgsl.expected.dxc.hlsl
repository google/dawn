
static int t = int(0);
int4 m() {
  t = int(1);
  return int4((t).xxxx);
}

[numthreads(1, 1, 1)]
void f() {
  uint4 v = uint4(m());
}

