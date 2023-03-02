[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

typedef int tint_workgroupUniformLoad_ret[4];
tint_workgroupUniformLoad_ret tint_workgroupUniformLoad(inout int p[4]) {
  GroupMemoryBarrierWithGroupSync();
  const int result[4] = p;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

groupshared int v[4];

typedef int foo_ret[4];
foo_ret foo() {
  return tint_workgroupUniformLoad(v);
}
