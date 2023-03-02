[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

typedef int tint_workgroupUniformLoad_ret[128];
tint_workgroupUniformLoad_ret tint_workgroupUniformLoad(inout int p[128]) {
  GroupMemoryBarrierWithGroupSync();
  const int result[128] = p;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

groupshared int v[128];

int foo() {
  const int tint_symbol[128] = tint_workgroupUniformLoad(v);
  return tint_symbol[0];
}
