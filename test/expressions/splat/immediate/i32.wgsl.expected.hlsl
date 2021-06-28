[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  int2 v2 = int2((1).xx);
  int3 v3 = int3((1).xxx);
  int4 v4 = int4((1).xxxx);
}
