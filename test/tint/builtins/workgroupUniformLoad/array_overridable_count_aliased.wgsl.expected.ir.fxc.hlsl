
groupshared int v[128];
int foo() {
  GroupMemoryBarrierWithGroupSync();
  int v_1[128] = v;
  GroupMemoryBarrierWithGroupSync();
  int v_2[128] = v_1;
  return v_2[int(0)];
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

