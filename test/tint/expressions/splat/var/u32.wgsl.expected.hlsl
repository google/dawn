[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  uint v = (1u + 2u);
  uint2 v2 = uint2((v).xx);
  uint3 v3 = uint3((v).xxx);
  uint4 v4 = uint4((v).xxxx);
}
