[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  uint2 v2 = uint2((1u).xx);
  uint3 v3 = uint3((1u).xxx);
  uint4 v4 = uint4((1u).xxxx);
}
