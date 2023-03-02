[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int tint_workgroupUniformLoad(inout int p) {
  GroupMemoryBarrierWithGroupSync();
  const int result = p;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

groupshared int v[4];

int foo_v_X(uint p[1]) {
  return tint_workgroupUniformLoad(v[p[0]]);
}

int bar() {
  const uint tint_symbol[1] = (uint[1])0;
  return foo_v_X(tint_symbol);
}
