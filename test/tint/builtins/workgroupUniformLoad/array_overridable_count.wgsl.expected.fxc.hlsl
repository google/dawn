[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

groupshared int v[128];

int tint_workgroupUniformLoad_v_X(uint p[1]) {
  GroupMemoryBarrierWithGroupSync();
  int result = v[p[0]];
  GroupMemoryBarrierWithGroupSync();
  return result;
}

int foo() {
  uint tint_symbol[1] = (uint[1])0;
  return tint_workgroupUniformLoad_v_X(tint_symbol);
}
