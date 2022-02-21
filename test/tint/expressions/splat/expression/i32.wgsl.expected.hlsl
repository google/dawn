[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  int2 v2 = int2(((1 + 2)).xx);
  int3 v3 = int3(((1 + 2)).xxx);
  int4 v4 = int4(((1 + 2)).xxxx);
}
