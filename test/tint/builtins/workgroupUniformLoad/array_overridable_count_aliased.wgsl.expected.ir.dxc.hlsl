
groupshared int v[128];
int foo() {
  GroupMemoryBarrierWithGroupSync();
  int v_1 = v[0u];
  GroupMemoryBarrierWithGroupSync();
  return v_1;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

