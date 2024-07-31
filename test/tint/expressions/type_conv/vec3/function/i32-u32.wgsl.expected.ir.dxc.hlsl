
static int t = 0;
int3 m() {
  t = 1;
  return int3((t).xxx);
}

void f() {
  uint3 v = uint3(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

